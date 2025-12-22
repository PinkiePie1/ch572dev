target("RF_Basic")
    set_kind("binary")
    set_toolchains("wch-riscv-gcc")
    add_deps("ch572")
    add_rules("generateAll")

    -- 添加源文件
    add_files("./*.c")

    -- 添加头文件搜索路径
    add_includedirs(
        "./"
    )

    add_ldflags(
	"-lCH57xRF",
	{force = true}
    )

    check_ld()



