/* application.vala
 *
 * Copyright 2022 Oleg Aushev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


namespace CanMonitor {


public class Application : Adw.Application
{
	public Application()
	{
		Object (application_id: "org.example.uCAN-Monitor", flags: ApplicationFlags.FLAGS_NONE);
	}

	construct
	{
		ActionEntry[] action_entries = {
			{"about", this.on_about_action},
			{"preferences", this.on_preferences_action},
			{"canbus_setup", this.on_canbus_setup_action},
			{"quit", this.quit},
			{"create_main_window", this.on_create_main_window_action}

		};
		this.add_action_entries(action_entries, this);
		this.set_accels_for_action("app.quit", {"<primary>q"});
        }

	public override void activate()
	{
		base.activate();
		var win = this.active_window;
		if (win == null)
		{
			win = new CanMonitor.WindowSelectServer(this);
		}
		win.present();
		message("[gui] Waiting for ucanopen server selection...");
        }

	private void on_about_action()
	{
		string[] authors = { "Oleg Aushev" };
		Gtk.show_about_dialog (this.active_window,
				"program-name", "uCAN Monitor",
				"authors", authors,
				"version", GIT_DESCRIBE);
	}

	private void on_preferences_action()
	{
		message ("app.preferences action activated");
	}

	private void on_canbus_setup_action()
	{
		var win = this.active_window;
		var win_canbus = new CanMonitor.WindowCanBusPreferences();
		win_canbus.set_transient_for(win);
		win_canbus.present();
	}

	private void on_create_main_window_action()
	{
		message(@"[gui] '$(Backend.Ucanopen.server)' is selected.");
		message("[gui] Waiting for backend...");
		Backend.main_enter();
		while (!Backend.is_ready)
		{
			Thread.usleep(10000);	// with empty loop Release build is not working
		}
		
		message("[gui] Configuring backend...");
		ucanopen_client_set_node_id(WindowCanBusPreferences.client_id);
		ucanopen_client_set_server_id(Backend.Ucanopen.server, WindowCanBusPreferences.server_id);
		ucanopen_client_set_tpdo_enabled(WindowCanBusPreferences.tpdo_state);
		ucanopen_client_set_sync_period(WindowCanBusPreferences.sync_period);
		ucanopen_client_set_sync_enabled(WindowCanBusPreferences.sync_state);
		ucanopen_client_set_watch_period(WindowCanBusPreferences.watch_period);
		ucanopen_client_set_watch_enabled(WindowCanBusPreferences.watch_state);
		message("[gui] Backend ready.");

		message(string.join(null, "Set locale to ", Intl.setlocale(ALL, "en_US.UTF-8"), "."));

		this.shutdown.connect(Backend.main_exit);

		var win = this.active_window;
		if (win == null)
		{
			win = new CanMonitor.Window(this);
		}
		win.present();
		message("[gui] GUI ready.");
	}
}


}


