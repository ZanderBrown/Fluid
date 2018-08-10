if get_option('with_vapi')
  example_resources = gnome.compile_resources(
    'example-resources',
    'example.gresources.xml',
    c_name: 'example',
  )

  example_application_sources = [
    'example.vala',
    example_resources,
  ]

  example_application = executable('example', example_application_sources,
    dependencies: [fluid_vapi, fluid_dep],
    install: false
  )
endif
