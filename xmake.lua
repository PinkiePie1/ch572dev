-- 设置交叉编译工具链
set_toolchains("riscv-gcc")

toolchain("riscv-gcc")
    set_kind("standalone")
    set_sdkdir("/mnt/c/MRStoolChain/RISC-V Embedded GCC12")
    set_toolset("cc", "riscv-wch-elf-gcc")
    set_toolset("as", "riscv-wch-elf-gcc")
    set_toolset("ld", "riscv-wch-elf-gcc")
    set_toolset("ar", "riscv-wch-elf-ar")
    set_toolset("objcopy", "riscv-wch-elf-objcopy")
    set_toolset("size", "riscv-wch-elf-size")
    set_toolset("objdump", "riscv-wch-elf-objdump")
toolchain_end()

-- 构建目标
target("ch585")
    set_kind("binary")
    set_toolchains("riscv-gcc")
    set_targetdir("objs")

    add_rules("c.unity_build")

    -- 添加源文件
    add_files("*.c")
    add_files("CH585Libs/StdPeriphDriver/*.c")
    add_files("CH585Libs/Startup/startup_CH585.S")

    -- 添加头文件搜索路径
    add_includedirs(
        "./",
        "CH585Libs/StdPeriphDriver/inc",
        "CH585Libs/Startup",
        "CH585Libs/RVMSIS"
    )

    -- 设置链接脚本
    set_policy("build.merge_archive", false) -- 防止 .a 自动展开
    add_ldflags("-TCH585Libs/Ld/Link.ld", {force = true})

    -- 架构和优化
    local arch_flags = {
        "-march=rv32imc_zba_zbb_zbc_zbs_xw",
        "-mabi=ilp32",
        "-mcmodel=medany",
        "-msmall-data-limit=8",
        "-msave-restore",
        "-fmax-errors=20",
        "-fmessage-length=0",
        "-fsigned-char",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        "-Os"
    }

    set_policy("check.auto_ignore_flags", false)

    add_cflags(table.unpack(arch_flags))
    add_asflags(table.unpack(arch_flags), "-x", "assembler-with-cpp")
    -- add_defines("__GNUC__") -- 或你用到的 C_DEFS 宏

    -- 设置链接参数
    add_ldflags(
        table.unpack(arch_flags),
        "-nostartfiles",
        "-Xlinker", "--gc-sections",
 --       "-Xlinker", "--print-memory-usage",
 --       "-Wl,-Map=objs/ch585.map",
        "--specs=nano.specs",
        "--specs=nosys.specs",
        "-Wl,--wrap=memcpy",
        "-LCH585Libs/BLELIB",
        "-LCH585Libs/StdPeriphDriver",
        "-L../",
        "-lprintf",
        "-lCH58xBLE",
        "-lISP585",
        {force = true}
    )


    -- 生成 bin 和 hex
    after_build(function (target)
        import("core.tool.toolchain")
        -- 加载你刚才定义的 riscv-gcc toolchain
        local gcc = target:tool("cc")
        local objpath , _ = path.filename(gcc):gsub("gcc", "objcopy")
        local objcopy = path.join(path.directory(gcc), objpath)
        os.mkdir("objs")
        os.execv(objcopy, {
            "-O", "binary",
            "-S", target:targetfile(),
            path.join("objs", target:name() .. ".bin")
        })
    end)


target("CH572libs")
    set_kind("static")
    add_files("CH572libs/StdPeriphDriver/*.c")
    add_files("CH572libs/Startup/startup_CH572.S")
    

    add_includedirs(
        "./",
        "CH572libs/StdPeriphDriver/inc",
        "CH572libs/Startup",
        "CH572libs/RVMSIS"
    )


target("ch572_blink")
    set_kind("binary")
    set_toolchains("riscv-gcc")
    add_deps("CH572libs")

 --   add_rules("c.unity_build")

    -- 添加源文件
    add_files("ch572_examples/blink/*.c")
    add_files("CH572libs/StdPeriphDriver/*.c")
    add_files("CH572libs/Startup/startup_CH572.S")

    -- 添加头文件搜索路径
    add_includedirs(
        "./ch572_examples/blink",
        "CH572libs/StdPeriphDriver/inc",
        "CH572libs/Startup",
        "CH572libs/RVMSIS"
    )

    -- 设置链接脚本
    set_policy("build.merge_archive", false) -- 防止 .a 自动展开
    add_ldflags("-TCH572libs/Ld/Link.ld", {force = true})

    -- 架构和优化
    local arch_flags = {
        "-march=rv32imc_zba_zbb_zbc_zbs_xw",
        "-mabi=ilp32",
        "-mcmodel=medany",
        "-msmall-data-limit=8",
        "-msave-restore",
        "-fmax-errors=20",
        "-fmessage-length=0",
        "-fsigned-char",
        "-ffunction-sections",
        "-fdata-sections",
        "-fno-common",
        "-Os"
    }

    set_policy("check.auto_ignore_flags", false)

    add_cflags(table.unpack(arch_flags))
    add_asflags(table.unpack(arch_flags), "-x", "assembler-with-cpp")
    -- add_defines("__GNUC__") -- 或你用到的 C_DEFS 宏

    -- 设置链接参数
    add_ldflags(
        table.unpack(arch_flags),
        "-nostartfiles",
        "-Xlinker --gc-sections",
        "-Xlinker --print-memory-usage",
        "-Wl,-Map=build/ch572_blink.map",
        "--specs=nano.specs",
        "--specs=nosys.specs",
        "-Wl,--wrap=memcpy",
        "-LCH572libs/StdPeriphDriver",
        "-L../",
        "-lprintf",
        "-lISP572",
        {force = true}
    )


    -- 生成 bin 和 hex
    after_build(function (target)
        import("core.tool.toolchain")
        -- 加载你刚才定义的 riscv-gcc toolchain
        local gcc = target:tool("cc")
        local objpath , _ = path.filename(gcc):gsub("gcc", "objcopy")
        local objcopy = path.join(path.directory(gcc), objpath)
        os.mkdir("objs")
        os.execv(objcopy, {
            "-O", "binary",
            "-S", target:targetfile(),
            path.join("objs", target:name() .. ".bin")
        })

        os.execv(objcopy, {
            "-O", "ihex",
            "-S", target:targetfile(),
            path.join("objs", target:name() .. ".hex")
        })
    end)