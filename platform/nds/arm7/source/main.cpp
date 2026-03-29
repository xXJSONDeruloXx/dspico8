/*---------------------------------------------------------------------------------

	default ARM7 core

	Copyright (C) 2005 - 2010
	Michael Noland (joat)
	Jason Rogers (dovoto)
	Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <calico.h>
#include <nds.h>
#include <maxmod7.h>

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

	soundStartServer(MAIN_THREAD_PRIO - 0x10);
	micStartServer(MAIN_THREAD_PRIO - 0x18);
	wlmgrStartServer(MAIN_THREAD_PRIO - 8);

	mmInstall(MAIN_THREAD_PRIO + 1);

	while (pmMainLoop()) {
		threadWaitForVBlank();
	}

	return 0;
}
