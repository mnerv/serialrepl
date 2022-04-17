project 'serialrepl'
  kind 'ConsoleApp'
  language 'C++'
  staticruntime 'On'
  cppdialect 'C++17'

  warnings 'extra'
  enablewarnings {
    'all'
  }
  disablewarnings {
    'deprecated-declarations'
  }

  targetdir('%{wks.location}/bin/%{cfg.buildcfg}')
  objdir('%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}')

  files {
    'src/**.h',
    'src/**.hpp',
    'src/**.cpp',
  }

  local asio_path = os.getenv('ASIO_PATH')

  includedirs {
    'src',
    -- Include ASIO Library
    asio_path..'/include'
  }

  filter {'not system:windows'}
    buildoptions {
      '-pedantic',
      '-Werror'
    }

  filter 'system:macosx'
    system 'macosx'

  filter 'system:linx'
    system "linux"

  filter 'system:windows'
    system "windows"

    defines {
        '_WIN32_WINNT 0x0A00'
    }

  filter 'configurations:debug'
    defines {'_DEBUG'}
    symbols 'On'

  filter 'configurations:release'
    defines {'_RELEASE'}
    optimize 'On'

