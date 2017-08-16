{
  'variables': {
    'target_arch%': 'ia32',          # set v8's target architecture, ia32, x64
  },
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'cflags': [ '-ggdb3', '-O0' ],
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
            'msvs_configuration_attributes' : {
              'OutputDirectory' : '$(SolutionDir)$(PlatformName)\\$(ConfigurationName)\\',
              'IntermediateDirectory' : '$(PlatformName)\\$(ConfigurationName)\\obj\\$(ProjectName)\\',
            },
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 3, # static debug
            'Optimization': 0, # /Od, no optimization
            'MinimalRebuild': 'false',
            'OmitFramePointers': 'false',
            'BasicRuntimeChecks': 3, # /RTC1
          },
          'VCLinkerTool': {
            'LinkIncremental': 2, # enable incremental linking
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'cflags': [ '-O2', '-g', '-ffunction-sections', '-fdata-sections' ],
        'conditions': [
          ['target_arch=="x64"', {
            'msvs_configuration_platform': 'x64',
            'msvs_configuration_attributes' : {
              'OutputDirectory' : '$(SolutionDir)$(PlatformName)\\$(ConfigurationName)\\',
              'IntermediateDirectory' : '$(PlatformName)\\$(ConfigurationName)\\obj\\$(ProjectName)\\',
            },
          }],
        ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 2, # static release
            'Optimization': 3, # /Ox, full optimization
            'FavorSizeOrSpeed': 1, # /Ot, favour speed over size
            'InlineFunctionExpansion': 2, # /Ob2, inline anything eligible
            'OmitFramePointers': 'false',
            'EnableFunctionLevelLinking': 'true',
            'EnableIntrinsicFunctions': 'true',
            'RuntimeTypeInfo': 'true',
            'ExceptionHandling': '1',
          },
          'VCLinkerTool': {
            'OptimizeReferences': 1, # /OPT:REF, to avoid ffmpeg runtime error
            'EnableCOMDATFolding': 2, # /OPT:ICF
            'LinkIncremental': 1, # disable incremental linking
          },
        },
      },
    },
    'msvs_settings': {
      'VCCLCompilerTool': {
        'StringPooling': 'true', # pool string literals
        'WarningLevel': 3,
        'BufferSecurityCheck': 'true',
        'ExceptionHandling': 1, # /EHsc
        'SuppressStartupBanner': 'true',
        'WarnAsError': 'false',
      },
      'VCLibrarianTool': {
        'conditions': [
          ['target_arch=="x64"', {
            'TargetMachine' : 17 # /MACHINE:X64
          }],
          ['target_arch=="ia32"', {
            'TargetMachine' : 1 # /MACHINE:X86
          }],
        ],
      },
      'VCLinkerTool': {
        'conditions': [
          ['target_arch=="x64"', {
            'TargetMachine' : 17 # /MACHINE:X64
          }],
          ['target_arch=="ia32"', {
            'TargetMachine' : 1 # /MACHINE:X86
          }],
        ],
        'GenerateDebugInformation': 'true',
        'RandomizedBaseAddress': 2, # enable ASLR
        'DataExecutionPrevention': 2, # enable DEP
        'AllowIsolation': 'true',
        'SuppressStartupBanner': 'true',
        'target_conditions': [
          ['_type=="executable"', {
            'SubSystem': 1, # console executable
          }],
        ],
      },
    },
    'conditions': [
      ['OS=="win"', {
        'msvs_cygwin_shell': 0, # prevent actions from trying to use cygwin
        'defines': [
          'WIN32',
          'WINDOWS',
          '_WIN32',
          # we don't really want VC++ warning us about
          # how dangerous C functions are...
          '_CRT_SECURE_NO_DEPRECATE',
          # ... or that C implementations shouldn't use
          # POSIX names
          '_CRT_NONSTDC_NO_DEPRECATE',

          '_CRT_SECURE_NO_WARNINGS',
          '_CRT_NONSTDC_NO_WARNINGS',
          '_SCL_SECURE_NO_WARNINGS',
          '_SCL_SECURE_NO_DEPRECATE',

          '_WINSOCK_DEPRECATED_NO_WARNINGS',
        ],
      }],
      ['OS=="linux"', {
        'defines': ['_FILE_OFFSET_BITS=64'],
        'cflags': [ '-pthread', '-pipe', '-Wall', '-fPIC', '-Wno-unused-local-typedefs' ],
        'cflags_cc': [ '-std=c++11' ],
        'ldflags': [ '-pthread', '-Wl,--no-as-needed' ],
      }],
      [ 'target_arch=="ia32"', {
        'cflags': [ '-m32' ],
        'ldflags': [ '-m32' ],
      }],
      [ 'target_arch=="x64"', {
        'cflags': [ '-m64' ],
        'ldflags': [ '-m64' ],
      }],
    ]
  },
}