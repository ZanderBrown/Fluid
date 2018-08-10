project('fluid', 'c', 'vala',
  version: '0.1.0',
  meson_version: '>= 0.40.0',
)

package_subdir = get_option('package_subdir') # When used as subproject
libdir = join_paths(get_option('libdir'), package_subdir)
girdir = join_paths(get_option('datadir'), package_subdir, 'gir-1.0')
typelibdir = join_paths(get_option('libdir'), package_subdir, 'girepository-1.0')
if package_subdir != ''
  bindir = get_option('bindir')
  vapidir = join_paths(get_option('datadir'), package_subdir, 'vapi')
else
  # Use libexecdir to avoid subdirectories under bindir
  bindir = join_paths(get_option('libexecdir'), package_subdir)
  vapidir = join_paths(get_option('datadir'), 'vala', 'vapi')
endif

config_h = configuration_data()
configure_file(
  output: 'fluid-config.h',
  configuration: config_h,
)
add_project_arguments([
  '-I' + meson.build_root(),
], language: 'c')

gnome = import('gnome')

subdir('src')
subdir('examples')

