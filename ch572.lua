
-- 设置交叉编译工具链
set_toolchains("wch-riscv-gcc")

toolchain("wch-riscv-gcc")
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

target("ch572")
	set_kind("object")

	--库文件
	add_files("CH572libs/StdPeriphDriver/*.c")
	add_files("CH572libs/StdPeriphDriver/*.a")
	add_files("CH572libs/Startup/startup_CH572.S")
	add_includedirs(
	    "./",
	    "CH572libs/StdPeriphDriver/inc",
	    "CH572libs/Startup",
	    "CH572libs/RVMSIS",
		"CH572libs/BLELIB",
	    {public = true}
	    )
	    
	arch_flags = {
	    "-march=rv32imc_zba_zbb_zbc_zbs_xw",
	    "-mabi=ilp32",
	    "-mcmodel=medany",
	    "-msmall-data-limit=8",
	    "-mno-save-restore",
	    "-fmax-errors=20",
		"-Os",
	    "-fmessage-length=0",
	    "-fsigned-char",
	    "-ffunction-sections",
	    "-fdata-sections",
	    "-fno-common",
		"--param=highcode-gen-section-name=1",
		"-g",
	    
	}

	

	add_ldflags(
	    arch_flags,
	    "-nostartfiles",
	    "-Xlinker --gc-sections",
	    "-Xlinker --print-memory-usage",
	    "-Wl,-Map=build/ch572.map",
	    "-Wl,--wrap=memcpy",
	    "--specs=nano.specs",
	    "--specs=nosys.specs",
	    "-LCH572libs/StdPeriphDriver",
		"-LCH572libs/BLELIB",
	    "-L../",
		"-lm",
		"-lISP572",
--	    "-T", "$(ldfile)",
--	    "-lprintf",
		{force = true}

	)
	
	set_policy("check.auto_ignore_flags", false)

	add_cflags(arch_flags,{public = true})
	add_cflags("-std=gnu17")
	add_asflags(arch_flags, "-x", "assembler-with-cpp")

	if has_config("debug") then
		add_cflags("-DDEBUG=0")
	end


-- 生成bin和hex文件
rule("generateAll")
    after_build(function (target,opt)
        import("core.tool.toolchain")
        import("utils.progress")
        os.mkdir("objs")
        os.execv(target:tool("objcopy"), {
            "-O", "binary",
            "-S", target:targetfile(),
            path.join("objs", target:name() .. ".bin")
        })
        progress.show(opt.progress, "${color.build.object}generated %s", target:name() .. ".bin")

        os.execv(target:tool("objcopy"), {
            "-O", "ihex",
            "-S", target:targetfile(),
            path.join("objs", target:name() .. ".hex")
        })
        progress.show(opt.progress, "${color.build.object}generated %s", target:name() .. ".hex")

		os.execv(target:tool("size"),{
			target:targetfile()
		})

		if has_config("unbrick") then
			os.execv("echo",{"unbricking..."})
			os.execv("./minichlink",{"-u"})
		end

		if has_config("flash") then
			progress.show(opt.progress, "${color.build.object}flashing %s", target:name())
			os.execv("./minichlink",{"-w", path.join("objs", target:name() .. ".bin") , "flash", "-b"})
		end
    end)

option("flash")
	set_default(false)
	set_showmenu(true)
	set_description("flash to mcu")

option("unbrick")
	set_default(false)
	set_showmenu(true)
	set_description("unbrick before flashing to mcu.")


option_end()
