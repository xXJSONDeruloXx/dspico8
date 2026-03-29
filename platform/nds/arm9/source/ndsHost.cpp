#include <fat.h>
#include <maxmod9.h>
#include <nds.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>


#include "../../../../source/host.h"
#include "../../../../source/hostVmShared.h"
#include "../../../../source/PicoRam.h"
#include "../../../../source/filehelpers.h"

#define SAMPLERATE (22050)
#define SAMPLESPERBUF (800)

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;

const int PicoFbLength = 128 * 64;

volatile int framesSinceWait = 0;

u32 kDown;
u32 kHeld;
u16 touchX;
u16 touchY;
uint8_t mouseBtnState;

Audio *_audio;
uint16_t _rgb555Colors[144];
u16 _frameExpandLut[256];
int targetFps = 60;

static mm_word onStreamRequest(mm_word length, mm_addr dest, mm_stream_formats format) {
	(void)format;
	if (_audio != nullptr) {
		_audio->FillMonoAudioBuffer((void*)dest, 0, length);
	}
	return length;
}

int consoleModel = 0;

void vblankHandler() {
	framesSinceWait++;
}

void initFrameExpandLut() {
	for (int i = 0; i < 256; i++) {
		_frameExpandLut[i] = static_cast<u16>(i & 0x0f)
			| (static_cast<u16>((i >> 4) & 0x0f) << 8);
	}
}

uint8_t ConvertInputToP8(uint32_t input) {
	uint8_t result = 0;
	if (input & KEY_LEFT) {
		result |= P8_KEY_LEFT;
	}

	if (input & KEY_RIGHT) {
		result |= P8_KEY_RIGHT;
	}

	if (input & KEY_UP) {
		result |= P8_KEY_UP;
	}

	if (input & KEY_DOWN) {
		result |= P8_KEY_DOWN;
	}

	if (input & KEY_B) {
		result |= P8_KEY_O;
	}

	if (input & KEY_A) {
		result |= P8_KEY_X;
	}

	if (input & KEY_START) {
		result |= P8_KEY_PAUSE;
	}

	return result;
}

void setRenderParamsFromStretch(StretchOption stretch) {
	lcdMainOnTop();
	switch(stretch) {
		case AltScreenPixelPerfect:
			lcdMainOnBottom();
		case PixelPerfect:
		case PixelPerfectStretch:
		case FourByThreeVertPerfect:
			bgSetScroll(3, -64, -32);
			bgSetScale(3, 0x100, 0x100);
			bgUpdate();
			break;
		case AltScreenStretch:
			lcdMainOnBottom();
		case StretchToFit:
		case FourByThreeStretch:
			bgSetScroll(3, -21, 0);
			bgSetScale(3, 0x0AC, 0x0AC);
			bgUpdate();
			break;
		case StretchToFill:
			bgSetScroll(3, 0, 0);
			bgSetScale(3, 0x080, 0x0AC);
			bgUpdate();
			break;
		case StretchAndOverflow:
			bgSetScroll(3, 0, 32);
			bgSetScale(3, 0x080, 0x080);
			bgUpdate();
			break;
	}
}

void audioSetup() {
	mm_ds_system sys;
	sys.mod_count = 0;
	sys.samp_count = 0;
	sys.mem_bank = 0;
	mmInit(&sys);

	mm_stream stream;
	stream.sampling_rate = SAMPLERATE;
	stream.buffer_length = SAMPLESPERBUF;
	stream.callback = onStreamRequest;
	stream.format = MM_STREAM_16BIT_MONO;
	stream.timer = MM_TIMER0;
	stream.manual = true;
	mmStreamOpen(&stream);
}

// void fifoPrinter(u32 value32, void *userdata) {
// 	printf("7=>%lX\n", value32);
// }

Host::Host(int windowWidth, int windowHeight) {
	(void)windowWidth;
	(void)windowHeight;
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
	bgInit(3, BgType_Bmp8, BgSize_B8_128x128, 0, 0);
	consoleInit(nullptr, 0, BgType_Text4bpp, BgSize_T_256x256, 0, 1, false, true);

	if(!fatInitDefault()) {
		printf("FAT init failed.\n");

		while(1) {
			swiWaitForVBlank();
		}
	}

	chdir(isDSiMode() ? "sd:/" : "fat:/"); // todo

	_logFilePrefix = "/_nds/dspico8/";

	_cartDirectory = "/p8carts";

	int res = chdir("/");

	if (res == 0 && access("/_nds", F_OK) != 0) {
		res = mkdir("/_nds", 0777);
	}

	if (res == 0 && access(_logFilePrefix.c_str(), F_OK) != 0) {
		res = mkdir(_logFilePrefix.substr(0, _logFilePrefix.size() - 1).c_str(), 0777);
	}

	std::string cartdatadir = _logFilePrefix + "cdata";
	if (res == 0 && access(cartdatadir.c_str(), F_OK) != 0) {
		res = mkdir(cartdatadir.c_str(), 0777);
	}

	if (res == 0 && access(_cartDirectory.c_str(), F_OK) != 0) {
		res = mkdir(_cartDirectory.c_str(), 0777);
	}

	// fifoSetValue32Handler(FIFO_USER_01, fifoPrinter, nullptr); // debug
}

void Host::setPlatformParams(
		int windowWidth,
		int windowHeight,
		uint32_t sdlWindowFlags,
		uint32_t sdlRendererFlags,
		uint32_t sdlPixelFormat,
		std::string logFilePrefix,
		std::string customBiosLua,
		std::string cartDirectory) {}


void Host::oneTimeSetup(Audio *audio) {
	stretch = PixelPerfect;

	_audio = audio;
	audioSetup();
	initFrameExpandLut();

	for(int i = 0; i < 144; i++) {
		_rgb555Colors[i] = lroundf(_paletteColors[i].Red * 31.0 / 255.0) | lroundf(_paletteColors[i].Green * 31 / 255) << 5 | lroundf(_paletteColors[i].Blue * 31 / 255) << 10;
	}

	irqSet(IRQ_VBLANK, vblankHandler);
	irqEnable(IRQ_VBLANK);

	loadSettingsIni();

	mouseOffsetX = 64;
	mouseOffsetY = 32;
	scaleX = 1.0f;
	scaleY = 1.0f;

	setRenderParamsFromStretch(stretch);
}

void Host::oneTimeCleanup() {
	saveSettingsIni();
}

void Host::setTargetFps(int fps) {
	targetFps = fps;
}

void Host::waitForTargetFps() {
	mmStreamUpdate();

	while (framesSinceWait < 60 / targetFps) {
		swiWaitForVBlank();
	}

	framesSinceWait = 0;
}

void Host::changeStretch() {
	if ((kDown & KEY_SELECT) && resizekey == YesResize) {
		switch(stretch) {
			case PixelPerfect:
			case PixelPerfectStretch:
			case FourByThreeVertPerfect:
				forceStretch(StretchToFit);
				break;
			case StretchToFit:
			case FourByThreeStretch:
				forceStretch(StretchToFill);
				break;
			case StretchToFill:
				forceStretch(StretchAndOverflow);
				break;
			case StretchAndOverflow:
				forceStretch(AltScreenPixelPerfect);
				break;
			case AltScreenPixelPerfect:
				forceStretch(AltScreenStretch);
				break;
			case AltScreenStretch:
				forceStretch(PixelPerfect);
				break;
		}
	}
}

void Host::forceStretch(StretchOption newStretch) {
	stretch = newStretch;

	switch(stretch) {
		case PixelPerfect:
		case PixelPerfectStretch:
		case FourByThreeVertPerfect:
			mouseOffsetX = 64;
			mouseOffsetY = 32;
			scaleX = 1.0f;
			scaleY = 1.0f;
			break;
		case StretchToFit:
		case FourByThreeStretch:
			mouseOffsetX = 32;
			mouseOffsetY = 0;
			scaleX = 0.6701571f;
			scaleY = 0.6666667f;
			break;
		case StretchToFill:
			mouseOffsetX = 0;
			mouseOffsetY = 0;
			scaleX = 0.5f;
			scaleY = 0.6666667f;
			break;
		case StretchAndOverflow:
			mouseOffsetX = 0;
			mouseOffsetY = -64;
			scaleX = 0.5f;
			scaleY = 0.5f;
			break;
		case AltScreenPixelPerfect:
			mouseOffsetX = 64;
			mouseOffsetY = 32;
			scaleX = 1.0f;
			scaleY = 1.0f;
			break;
		case AltScreenStretch:
			mouseOffsetX = 32;
			mouseOffsetY = 0;
			scaleX = 0.6701571f;
			scaleY = 0.6666667f;
			break;
	}

	setRenderParamsFromStretch(stretch);
}

InputState_t Host::scanInput() {
	scanKeys();

	kDown = keysDown();
	kHeld = keysHeld();

	touchPosition touch;

	//Read the touch screen coordinates
	touchRead(&touch);

	if (touch.px > 0 && touch.py > 0) {
		touchX = (touch.px - mouseOffsetX) * scaleX;
		touchY = (touch.py - mouseOffsetY) * scaleY;
		mouseBtnState = 1;
	} else {
		mouseBtnState = 0;
	}

	return InputState_t {
		ConvertInputToP8(kDown),
		ConvertInputToP8(kHeld),
		(int16_t)touchX,
		(int16_t)touchY,
		mouseBtnState
	};
}

bool Host::shouldQuit() {
	u32 keys = KEY_L | KEY_R;

	return (kHeld & keys) == keys;
}

void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode) {
	(void)drawMode;

	for(int i = 0; i < 16; i++) {
		BG_PALETTE[i] = _rgb555Colors[screenPaletteMap[i]];
	}

	const uint8_t* src = picoFb;
	u16* dst = BG_GFX;
	for (int i = 0; i < PicoFbLength; i++) {
		dst[i] = _frameExpandLut[src[i]];
	}

	// switch(drawMode) {
	// 	case 1:
	// 		screenModeScaleX = 2.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	case 2:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 2.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	case 3:
	// 		screenModeScaleX = 2.0f;
	// 		screenModeScaleY = 2.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	//todo: mirroring- not sure how to do this?
	// 	//case 5,6,7

	// 	case 129:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = -1;
	// 		flipVertical = 1;
	// 		break;
	// 	case 130:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = 1;
	// 		flipVertical = -1;
	// 		break;
	// 	case 131:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = -1;
	// 		flipVertical = -1;
	// 		break;
	// 	case 133:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 1.5707963267949f; // pi / 2 (90 degrees)
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	case 134:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 3.1415926535898f; //pi (180 degrees)
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	case 135:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 4.7123889803847f; // pi * 3 / 2 (270 degrees)
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// 	default:
	// 		screenModeScaleX = 1.0f;
	// 		screenModeScaleY = 1.0f;
	// 		screenModeAngle = 0;
	// 		flipHorizontal = 1;
	// 		flipVertical = 1;
	// 		break;
	// }
}

bool Host::shouldFillAudioBuff() {
	return false;
}

void *Host::getAudioBufferPointer() {
	return nullptr;
}

size_t Host::getAudioBufferSize() {
	return 0;
}

void Host::playFilledAudioBuffer() { }

bool Host::shouldRunMainLoop() {
	return true;
}

std::vector<std::string> Host::listcarts() {
	std::vector<std::string> carts;

	chdir("/");

	DIR* dir = opendir(_cartDirectory.c_str());
	struct dirent *ent;

	if (dir) {
		while ((ent = readdir(dir)) != NULL) {
			if (isCartFile(ent->d_name)) {
				carts.push_back(_cartDirectory + "/" + ent->d_name);
			}
		}
		closedir(dir);
	}

	return carts;
}

const char* Host::logFilePrefix() {
	return _logFilePrefix.c_str();
}

std::string Host::customBiosLua() {
	return "";
}

std::string Host::getCartDirectory() {
	return _cartDirectory;
}

std::vector<std::string> Host::listdirs() {
	std::vector<std::string> dirs;

	chdir("/");

	DIR* dir = opendir(_cartDirectory.c_str());
	struct dirent* ent;

	if (dir) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_name[0] == '.') {
				continue;
			}

			std::string fullPath = _cartDirectory + "/" + ent->d_name;
			DIR* testDir = opendir(fullPath.c_str());
			if (testDir != NULL) {
				closedir(testDir);
				dirs.push_back(ent->d_name);
			}
		}
		closedir(dir);
	}

	return dirs;
}
