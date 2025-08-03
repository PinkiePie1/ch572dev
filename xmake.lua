set_policy("build.intermediate_directory", false)
--add_rules("c.unity_build")

function check_ld()
	local local_ld = os.scriptdir() .. "/link.ld"
	if os.isfile(local_ld) then
		add_ldflags("-T" .. local_ld, {force = true})
	else
		add_ldflags("-T","CH572libs/Ld/Link.ld",{force = true})
		add_cflags("-ICH572libs/Ld")
	end
end


includes("ch572.lua")

includes("**/xmake.lua")


