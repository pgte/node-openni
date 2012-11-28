{
  'targets': [
    {
      'target_name': 'openni',
      'sources': [
        'src/Context.cc',
      ],
      'include_dirs': [
        '/usr/include/ni'
      ],
      'libraries': [
        'libOpenNI.dylib',
      ],
    }
  ]
}