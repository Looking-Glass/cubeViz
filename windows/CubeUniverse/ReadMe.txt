
If your application uses MFC in a shared DLL, you will need
to redistribute the MFC DLLs. If your application is in a language
other than the operating system's locale, you will also have to
redistribute the corresponding localized resources MFC90XXX.DLL.
For more information on both of these topics, please see the section on
redistributing Visual C++ applications in MSDN documentation.

/////////////////////////////////////////////////////////////////////////////

有基本功能，还有一些要改进：
1. 把vcredist_x86.exe包含在软件里。
2. 除非用户按...这个选择程序的按钮选择了程序，否则都从网上下载新的程序，因为我们网上的那三个程序会定期更新。
3. 当系统找不到DFU设备时，要提示出错。提醒用户按按键进入DFU并安装驱动
4. DFU驱动包含在软件里点一下就安装。


遇到 IDB_PNG_README 编译不能通过时，删除，重新导入就可

libwdi.c
// extract the embedded binary resources
static int extract_binaries(const char* path)

/*
 * Return a wdi_device_info list of USB devices
 * parameter: driverless_only - boolean
 */
LIBWDI_EXP int LIBWDI_API wdi_create_list(struct wdi_device_info** list,
							   struct wdi_options_create_list* options);

/*
 * Release a wdi_device_info list allocated by the previous call
 */
LIBWDI_EXP int LIBWDI_API wdi_destroy_list(struct wdi_device_info* list);

/*
 * Create an inf file for a specific device
 */
LIBWDI_EXP int LIBWDI_API wdi_prepare_driver(struct wdi_device_info* device_info, const char* path,
								  const char* inf_name, struct wdi_options_prepare_driver* options);

/*
 * Install a driver for a specific device
 */
LIBWDI_EXP int LIBWDI_API wdi_install_driver(struct wdi_device_info* device_info, const char* path,
								  const char* inf_name, struct wdi_options_install_driver* options);
