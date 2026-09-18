if get_option('tests')
  resourceMonitorTest = executable(
    'resource-monitor-test',
    'core/resources/resource-monitor-test.cpp',
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  test('resource-monitor', resourceMonitorTest)
endif
