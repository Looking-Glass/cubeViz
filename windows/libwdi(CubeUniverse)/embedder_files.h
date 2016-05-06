/*
 * embedder : converts binary resources into a .h include
 * "If you can think of a better way to get ice, I'd like to hear it."
 * Copyright (c) 2010-2013 Pete Batard <pete@akeo.ie>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#pragma once

/*
 * This include defines the driver files that should be embedded in the library.
 * This file is meant to be used by libwdi developers only.
 * If you want to add extra files from a specific directory (eg signed inf and cat)
 * you should either define the macro USER_DIR in msvc/config.h (MS compilers) or
 * use the --with-userdir option when running configure.
 */

struct emb {
	int reuse_last;
	char* file_name;
	char* extraction_subdir;
};

#define _STR(s) #s
#define STR(s) _STR(s)

/*
 * files to embed
 */
#define		USE_CUBEDOCTOR_JP	//bywbj20160202

#ifdef USE_CUBEDOCTOR_JP
#define		DRV_DIR "D:/zadigfile/usb_driverVISTA"
#define		DRV_DIRX86 "D:/zadigfile/usb_driverVISTA/x86"
#define		DRV_DIRX64 "D:/zadigfile/usb_driverVISTA/amd64"

#define		DRV_XP_DIR "D:/zadigfile/usb_driverXP"
#define		DRV_XP_DIRX86 "D:/zadigfile/usb_driverXP/x86"
#define		DRV_XP_DIRX64 "D:/zadigfile/usb_driverXP/amd64"

#define		DDRV_DIR "usb_driverVISTA"
#define		DDRV_DIRX86 "usb_driverVISTA/x86"
#define		DDRV_DIRX64 "usb_driverVISTA/amd64"
#define		DDRV_XP_DIR "usb_driverXP"
#define		DDRV_XP_DIRX86 "usb_driverXP/x86"
#define		DDRV_XP_DIRX64 "usb_driverXP/amd64"

#define		SDRV_DIR "D:/zadigfile/CubeDoctor_JP_driver"
#define		SDRV_DFU_DIR "D:/zadigfile/CubeDoctor_JP_driver/dfu"
#define		DDRV_DFU_DIR "usb_driverDfu"
#define		SDRV_COM_DIR "D:/zadigfile/CubeDoctor_JP_driver/serial"
#define		DDRV_COM_DIR "usb_driverCom"
struct emb embeddable_fixed[] = {
	{ 0, DRV_DIRX86 "\\install-filter.exe", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.sys", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0_x86.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.sys", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\WdfCoInstaller01011.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\winusbcoinstaller2.dll", DDRV_DIRX86 },

	{ 0, DRV_DIRX64 "\\install-filter.exe", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.sys", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0_x86.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.sys", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK_x86.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\WdfCoInstaller01011.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\winusbcoinstaller2.dll", DDRV_DIRX64 },

	{ 0, DRV_DIR "\\license\\libusb0\\installer_license.txt", DDRV_DIR "\\license\\libusb0" },
	// Common files
	{ 0, DRV_DIR "\\installer_x86.exe", DDRV_DIR },
	{ 0, DRV_DIR "\\installer_x64.exe", DDRV_DIR },

	{ 0, DRV_DIRX86 "\\install-filter.exe", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.sys", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0_x86.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.sys", DDRV_XP_DIRX86 },
	{ 0, DRV_XP_DIRX86 "\\WdfCoInstaller01009.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\winusbcoinstaller2.dll", DDRV_XP_DIRX86 },

	{ 0, DRV_DIRX64 "\\install-filter.exe", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.sys", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0_x86.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.sys", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK_x86.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_XP_DIRX64 "\\WdfCoInstaller01009.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\winusbcoinstaller2.dll", DDRV_XP_DIRX64 },

	{ 0, DRV_DIR "\\license\\libusb0\\installer_license.txt", DDRV_XP_DIR "\\license\\libusb0" },
	// Common files
	{ 0, DRV_DIR "\\installer_x86.exe", DDRV_XP_DIR },
	{ 0, DRV_DIR "\\installer_x64.exe", DDRV_XP_DIR },
	//转串口驱动 "Photon with WiFi" VID=2B04,PID=C006

	{ 0, SDRV_COM_DIR "\\maple_serial.inf", DDRV_COM_DIR },
	{ 0, SDRV_COM_DIR "\\msports.dll", DDRV_COM_DIR },
	{ 0, SDRV_COM_DIR "\\serenum.sys", DDRV_COM_DIR },
	{ 0, SDRV_COM_DIR "\\usbser.sys", DDRV_COM_DIR },
	{ 0, SDRV_DIR "\\installer_x86.exe", DDRV_COM_DIR },
	{ 0, SDRV_DIR "\\installer_x64.exe", DDRV_COM_DIR },

	{ 0, SDRV_DFU_DIR "\\maple.inf", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\maple.cat", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\maple_x64.cat", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\libusb0.dll", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\libusb0.sys", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\libusb0_x64.dll", DDRV_DFU_DIR },
	{ 0, SDRV_DFU_DIR "\\libusb0_x64.sys", DDRV_DFU_DIR },
	{ 0, SDRV_DIR "\\installer_x86.exe", DDRV_DFU_DIR },
	{ 0, SDRV_DIR "\\installer_x64.exe", DDRV_DFU_DIR },

	// inf templates for the tokenizer ("" directory means no extraction)
	{ 0, "winusb.inf.in", "" },
	{ 0, "libusb0.inf.in", "" },
	{ 0, "libusbk.inf.in", "" },
	// cat file lists for self signing
	{ 0, "winusb.cat.in", "" },
	{ 0, "libusb0.cat.in", "" },
	{ 0, "libusbk.cat.in", "" },
};
#else
#define		DRV_DIR "D:/zadigfile/usb_driverVISTA"
#define		DRV_DIRX86 "D:/zadigfile/usb_driverVISTA/x86"
#define		DRV_DIRX64 "D:/zadigfile/usb_driverVISTA/amd64"

#define		DRV_XP_DIR "D:/zadigfile/usb_driverXP"
#define		DRV_XP_DIRX86 "D:/zadigfile/usb_driverXP/x86"
#define		DRV_XP_DIRX64 "D:/zadigfile/usb_driverXP/amd64"
#if 1	//释放时会分开释
#define		DDRV_DIR "usb_driverVISTA"
#define		DDRV_DIRX86 "usb_driverVISTA/x86"
#define		DDRV_DIRX64 "usb_driverVISTA/amd64"
#define		DDRV_XP_DIR "usb_driverXP"
#define		DDRV_XP_DIRX86 "usb_driverXP/x86"
#define		DDRV_XP_DIRX64 "usb_driverXP/amd64"
#endif
//转串口驱动
#define		USB2COMDRV_DIR "D:/zadigfile/usb2com_driver"
#define		USB2COMDDRV_DIR "usb2com_driver"

struct emb embeddable_fixed[] = {
	{ 0, DRV_DIRX86 "\\install-filter.exe", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.sys", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0_x86.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.sys", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\WdfCoInstaller01011.dll", DDRV_DIRX86 },
	{ 0, DRV_DIRX86 "\\winusbcoinstaller2.dll", DDRV_DIRX86 },

	{ 0, DRV_DIRX64 "\\install-filter.exe", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.sys", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0_x86.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.sys", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK_x86.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\WdfCoInstaller01011.dll", DDRV_DIRX64 },
	{ 0, DRV_DIRX64 "\\winusbcoinstaller2.dll", DDRV_DIRX64 },

	{ 0, DRV_DIR "\\license\\libusb0\\installer_license.txt", DDRV_DIR "\\license\\libusb0" },
// Common files
	{ 0, DRV_DIR "\\installer_x86.exe", DDRV_DIR },
	{ 0, DRV_DIR "\\installer_x64.exe", DDRV_DIR },

	{ 0, DRV_DIRX86 "\\install-filter.exe", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0.sys", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusb0_x86.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\libusbK.sys", DDRV_XP_DIRX86 },
	{ 0, DRV_XP_DIRX86 "\\WdfCoInstaller01009.dll", DDRV_XP_DIRX86 },
	{ 0, DRV_DIRX86 "\\winusbcoinstaller2.dll", DDRV_XP_DIRX86 },

	{ 0, DRV_DIRX64 "\\install-filter.exe", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0.sys", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusb0_x86.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK.sys", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\libusbK_x86.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_XP_DIRX64 "\\WdfCoInstaller01009.dll", DDRV_XP_DIRX64 },
	{ 0, DRV_DIRX64 "\\winusbcoinstaller2.dll", DDRV_XP_DIRX64 },

	{ 0, DRV_DIR "\\license\\libusb0\\installer_license.txt", DDRV_XP_DIR "\\license\\libusb0" },
	// Common files
	{ 0, DRV_DIR "\\installer_x86.exe", DDRV_XP_DIR },
	{ 0, DRV_DIR "\\installer_x64.exe", DDRV_XP_DIR },
	//转串口驱动 "Photon with WiFi" VID=2B04,PID=C006
	{ 0, DRV_DIR "\\installer_x86.exe", USB2COMDDRV_DIR },
	{ 0, DRV_DIR "\\installer_x64.exe", USB2COMDDRV_DIR },
	{ 0, USB2COMDRV_DIR "\\photon.cat", USB2COMDDRV_DIR },
	{ 0, USB2COMDRV_DIR "\\photon.inf", USB2COMDDRV_DIR },

// inf templates for the tokenizer ("" directory means no extraction)
	{ 0, "winusb.inf.in", "" },
	{ 0, "libusb0.inf.in", "" },
	{ 0, "libusbk.inf.in", "" },
// cat file lists for self signing
	{ 0, "winusb.cat.in", "" },
	{ 0, "libusb0.cat.in", "" },
	{ 0, "libusbk.cat.in", "" },
};
#endif
