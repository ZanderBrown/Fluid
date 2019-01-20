[GtkTemplate (ui = "/org/gnome/example/window.ui")]
class Win : Fld.ApplicationWindow {
    [GtkChild]
    Gtk.Label label;

    [GtkChild]
    Gtk.Stack content;

    [GtkChild]
    Gtk.Button rem;

    [GtkChild]
    Gtk.Button end;

    construct {
        bind_property ("compact", label, "visible");
        content.add (new Gtk.Label ("&"));
        content.add (new Gtk.Label ("%"));

        rem.clicked.connect (() => content.visible_child.destroy ());
        end.clicked.connect (() => {
            var curr = content.visible_child;
            Value title;
            content.child_get_property(curr, "title", ref title);
            content.remove (curr);
            content.add_titled (curr, "r%u".printf(Random.next_int ()), title as string);
            curr.show ();
        });
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
