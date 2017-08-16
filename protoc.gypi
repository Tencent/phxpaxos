{
  'variables': {
    'protoc%': '<(PRODUCT_DIR)/protoc<(EXECUTABLE_SUFFIX)',
    'protoc_output_dir%': '<(PRODUCT_DIR)/protoc_out',
  },
  'include_dirs': ['<(protoc_output_dir)'],
  'dependencies': [
    'protoc',
    'libprotobuf',
  ],
  'export_dependent_settings': [
    'libprotobuf',
  ],
  'direct_dependent_settings': {
    'include_dirs': ['<(protoc_output_dir)'],
  },
	'rules': [
    {
      'rule_name': 'protoc',
      'extension': 'proto',
      'outputs': [
        '<(protoc_output_dir)/<(RULE_INPUT_ROOT).pb.cc',
        '<(protoc_output_dir)/<(RULE_INPUT_ROOT).pb.h',
      ],
      'inputs': [
        '<(protoc)',
      ],
      'action': [
        '<(protoc)',
        '--cpp_out=<(protoc_output_dir)',
        '-I',
        '<(RULE_INPUT_DIRNAME)',
        '<(RULE_INPUT_PATH)',
      ],
      'msvs_cygwin_shell': 0,
      'message': 'Generating C++ code from <(RULE_INPUT_PATH)',
      'process_outputs_as_sources': 1,
    },
  ],
}