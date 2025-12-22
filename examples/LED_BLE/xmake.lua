target("LED_BLE")
	set_kind("binary")
    set_toolchains("wch-riscv-gcc")
    add_deps("ch572")
    add_rules("generateAll")

	add_files("./**.c")
	
	add_includedirs(
		"./APP/include",
		"./HAL/include",
		"./Profile/include"
	)
	check_ld()

add_ldflags(
	"-lCH572BLE_PERI",	
	{force = true}
)
