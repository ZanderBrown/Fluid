[GtkTemplate (ui = "/org/gnome/example/window.ui")]
class Win : Fld.ApplicationWindow {
    [GtkChild]
    Gtk.Label label;

    [GtkChild]
    Gtk.Stack content;

    construct {
        bind_property ("compact", label, "visible");
        content.add (new Gtk.Label ("&"));
        content.add (new Gtk.Label ("%"));
    }

    public Win (App app) {
        Object (application: app);
    }
}

class App : Gtk.Application {
    construct {
        application_id = "org.gnome.example";
        flags = FLAGS_NONE;
    }

    public override void activate () {
        new Win (this).present ();
    }

    public override void startup () {
        base.startup();
        var styles = new Gtk.CssProvider();
        styles.load_from_resource("/org/gnome/example/styles.css");
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), styles, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

int main (string[] args) {
    return new App ().run (args);
}
