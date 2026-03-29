#include <calico.h>
#include <nds.h>

int main() {
    envReadNvramSettings();
    keypadStartExtServer();

    lcdSetIrqMask(DISPSTAT_IE_ALL, DISPSTAT_IE_VBLANK);
    irqEnable(IRQ_VBLANK);

    rtcInit();
    rtcSyncTime();
    pmInit();
    blkInit();
    touchInit();
    touchStartServer(80, MAIN_THREAD_PRIO);

    while (pmMainLoop()) {
        threadWaitForVBlank();
    }

    return 0;
}
