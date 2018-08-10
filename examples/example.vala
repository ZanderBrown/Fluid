[GtkTemplate (ui = "/org/gnome/example/window.ui")]
class Win : Fld.ApplicationWindow {
    [GtkChild]
    Gtk.Label label;

    construct {
        bind_property ("compact", label, "visible");
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
}

int main (string[] args) {
    return new App ().run (args);
}
