target("ws2812")
    set_kind("binary")
    set_toolchains("wch-riscv-gcc")
    add_deps("ch572")
    add_rules("generateAll")

    -- 添加源文件
    add_files("*.c")

    -- 添加头文件搜索路径
    add_includedirs(
        "./"
    )

    check_ld()


    

--  add_defines("DEBUG=1") --这里放预处理define

