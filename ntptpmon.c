#include <nt.h>
#include <unistd.h>
#include <stdio.h>

/*
#define DEBUG 1
*/

static char prepend[18]="node_text_nt_ptp";

int GrandMasterId(uint8_t* address, char* gmId) {
  int i;
  char tmp[4] = "";
  for (i=0; i<8; i++) { 
    if (i > 0 ) {
      sprintf(tmp, ":%02x", address[i]);
    } else {
      sprintf(tmp, "%02x", address[i]);
    }
    strcat(gmId, tmp); 
  }
#ifdef DEBUG
  printf("%s\n", gmId);
#endif
}

int printOutput(int protocol, int status, uint16_t steps, char* gm, int64_t skew) {
    printf("Protocol: %s, %s, Hops: %i, GM: %s, Clock Skew: %li\n", 
      protocol == NT_TIMESYNC_REFERENCE_PTP ? "PTP":"NOT PTP!",
      status == NT_TIMESYNC_INSYNC_STATUS_IN_SYNC ? "In Sync":"NOT In Sync!",
      steps, gm, skew);
}         

int nodeExporter(int status, uint16_t steps, char* gm, int64_t skew) {
  char gminfo[40];
  FILE *fp;

  snprintf(gminfo, 39, "grand_master=\"%s\"", gm); 
  
  fp = fopen("/var/lib/node_exporter/textfiles/ntptp.prom", "w");
  fprintf(fp, "%s_in_sync{%s} %i\n", 
    prepend, gminfo, status == NT_TIMESYNC_INSYNC_STATUS_IN_SYNC ? 1:0);
  fprintf(fp, "%s_steps_from_gm{%s} %i\n", prepend, gminfo, steps);
  fprintf(fp, "%s_skew{%s} %i\n", prepend, gminfo, skew);

  fclose(fp);
}

int main(int argc, char **argv) {
  int status;
  int hardReset=0;
  int i;
  char *gmId;
  char tmp[4];

  static NtInfoStream_t hInfo=NULL;
  static NtConfigStream_t hConfig=NULL;
  NtInfo_t tsRead;
  NtInfo_t ptpRead;
  NtConfig_t tsConfig;

  int c;
  int aflag;

  while ((c = getopt (argc, argv, "e")) != -1) 
    switch (c) {
      case 'e':
        aflag = 1;  /* print node_exporter textfile every 30s*/
        break;
      default:
        aflag = 0;  /* print to stdout every 1s */
    } 

  if (NT_Init(NTAPI_VERSION) != NT_SUCCESS) {
    printf("NT_Init failed. Is ntservice started?\n");
    exit(1);
  }
  if((status = NT_InfoOpen(&hInfo, "hInfo")) != 0) {
    printf("NT_InfoOpen failed.\n");
    exit(1);
  }
  if((status = NT_ConfigOpen(&hConfig, "hConfig")) != 0) {
    printf("NT_ConfigOpen failed.\n");
    exit(1);
  }
  while (1) {
    gmId = (char *) malloc(24);
    tmp[0] = '\0';
    tsRead.cmd=NT_INFO_CMD_READ_TIMESYNC_V3;
    tsRead.u.timeSync_v3.adapterNo = 0;
    if  ((status = NT_InfoRead(hInfo, &tsRead)) != NT_SUCCESS) {
      printf("NT_InfoRead timeSync failed\n");
      exit(1);
    }
    if (!tsRead.u.timeSync_v3.data.timeSyncSupported) {
      printf("Timesync not supported on this adapter\n");
      exit(1);
    }
    if (tsRead.u.timeSync_v3.data.timeRef == NT_TIMESYNC_REFERENCE_FREE_RUN ||
        tsRead.u.timeSync_v3.data.timeRef == NT_TIMESYNC_REFERENCE_INVALID) {
      printf("Free running. No sync mechanism activated yet\n");
      sleep(2);
      continue;
    }
    ptpRead.cmd=NT_INFO_CMD_READ_PTP_V2;
    ptpRead.u.ptp_v2.adapterNo = 0;

    if ((status = NT_InfoRead(hInfo, &ptpRead)) != NT_SUCCESS) {
      printf("NT_InfoRead ptp failed\n");
      exit(1);
    } 
    
    GrandMasterId(ptpRead.u.ptp_v2.data.ptpDataSets.parentDs.gmId, gmId);

    if (aflag == 0) {
      printOutput(tsRead.u.timeSync_v4.data.timeRef,
        tsRead.u.timeSync_v4.data.timeSyncInSyncStatus,
        ptpRead.u.ptp_v2.data.ptpDataSets.currentDs.stepsRemoved,
        gmId,
        tsRead.u.timeSync_v4.data.timeSyncTimeSkew); 
        sleep(1);
    } else {
      nodeExporter(tsRead.u.timeSync_v4.data.timeSyncInSyncStatus,
        ptpRead.u.ptp_v2.data.ptpDataSets.currentDs.stepsRemoved,
        gmId,
        tsRead.u.timeSync_v4.data.timeSyncTimeSkew); 
        sleep(30);
    }
    free(gmId);
  }
  exit(0);
}
