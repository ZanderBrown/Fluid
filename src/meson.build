api_version = '0.1'

fluid_sources = [
  'fld-window.c'
]

fluid_headers = [
  'fluid.h',
  'fld-window.h'
]

version_split = meson.project_version().split('.')
MAJOR_VERSION = version_split[0]
MINOR_VERSION = version_split[1]
MICRO_VERSION = version_split[2]

version_conf = configuration_data()
version_conf.set('VERSION', meson.project_version())
version_conf.set('MAJOR_VERSION', MAJOR_VERSION)
version_conf.set('MINOR_VERSION', MINOR_VERSION)
version_conf.set('MICRO_VERSION', MICRO_VERSION)

configure_file(
  input: 'fluid-version.h.in',
  output: 'fluid-version.h',
  configuration: version_conf,
  install: true,
  install_dir: join_paths(get_option('includedir'), 'fluid')
)

fluid_deps = [
  dependency('gio-2.0', version: '>= 2.50'),
  dependency('gtk+-3.0'),
]

fluid_lib = shared_library('fluid-' + api_version,
  fluid_sources,
  dependencies: fluid_deps,
  install: true,
)

install_headers(fluid_headers, subdir: 'fluid')

fluid_dep = declare_dependency(
              sources: fluid_headers,
         dependencies: fluid_deps,
            link_with: fluid_lib,
  include_directories: include_directories('.'),
)

fluid_package = 'fluid-@0@'.format(api_version)

if get_option('with_introspection')

  fluid_gir = gnome.generate_gir(fluid_lib,
                sources: fluid_headers + fluid_sources,
              nsversion: api_version,
              namespace: 'Fluid',
          symbol_prefix: 'fld',
      identifier_prefix: 'Fld',
              link_with: fluid_lib,
               includes: ['Gio-2.0', 'Gtk-3.0'],
                install: true,
        install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
        export_packages: fluid_package,
             extra_args: [ '--c-include=fluid.h', '--quiet' ],
  )

  if get_option('with_vapi')

    fluid_vapi = gnome.generate_vapi(fluid_package,
            sources: fluid_gir[0],
           packages: [ 'gio-2.0', 'gtk+-3.0' ],
            install: true,
        install_dir: vapidir,
      metadata_dirs: [ meson.current_source_dir() ],
    )

  endif
endif

pkg = import('pkgconfig')

pkg.generate(
  description: 'Tools for fluid applications',
    libraries: fluid_lib,
         name: 'fluid',
     filebase: fluid_package,
      version: meson.project_version(),
      subdirs: 'fluid',
     requires: 'gtk+-3.0',
  install_dir: join_paths(get_option('libdir'), 'pkgconfig')
)

