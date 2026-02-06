#include <gtk/gtk.h>
#include <stdio.h>
#include <libnotify/notify.h>

#include "drives.h"
#include "notify.h"

typedef struct {
	char *node;
	char *model;
	PartitionInfo *parts;
	size_t part_count;
} DriveCallbackData;

static void on_power_off_clicked (GtkMenuItem *item, gpointer user_data) 
{
	DriveCallbackData *cb = user_data;
	printf("Power off %s (%s)\n", cb->model, cb->node);
	char body[256];
	snprintf(body, sizeof(body), "Powering off %s (%s)", cb->model, cb->node);
	NotifyNotification *n = send_notification("mountatray", body, "drive-harddisk");

	for (size_t i = 0; i < cb->part_count; i++) {
		if (cb->parts[i].mounted) {
			unmount_partition(cb->parts[i].node);
		}
	}

	power_off_drive(cb->node);
	snprintf(body, sizeof(body), "%s (%s) powered off", cb->model, cb->node);
	update_notification(n, "mountatray", body, "drive-harddisk");
	g_object_unref(n);
}

static void on_partition_clicked(GtkMenuItem *item, gpointer user_data)
{
	PartitionInfo *part = (PartitionInfo *)user_data;
	printf("Action on %s (%s)\n", part->label, part->node);

	char body[256];

	if (part->mounted) {
		unmount_partition(part->node);
		snprintf(body, sizeof(body), "%s (%s) unmounted", 
				part->label, part->node);
		send_notification("mountatray", body, "drive-harddisk");
	} else {
		mount_partition(part->node);
		snprintf(body, sizeof(body), "%s (%s) mounted", 
				part->label, part->node);
		send_notification("mountatray", body, "drive-harddisk");
	}
}

static void on_quit(GtkMenuItem *item, gpointer data) {
    gtk_main_quit();
}

static void show_menu(GtkStatusIcon *status_icon, guint button, guint time)
{
    GtkWidget *menu = gtk_menu_new();
	
	// list drives
    size_t count;
    DriveInfo *drives = get_external_drives(&count);

    if (count == 0) {
        GtkWidget *empty = gtk_menu_item_new_with_label("No external drives");
        gtk_widget_set_sensitive(empty, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), empty);
    } else {
		// loop drives
		for (size_t i = 0; i < count; i++) {
			char drive_label[256];
			snprintf(drive_label, sizeof(drive_label), "%s (%s)",
					drives[i].node, drives[i].model);

			// new drive menu item with sub menu
			GtkWidget *drive_item = gtk_image_menu_item_new_with_label(drive_label);
			GtkWidget *drive_image = gtk_image_new_from_icon_name(
				"drive-removable-media", GTK_ICON_SIZE_MENU);
			gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(drive_item), drive_image);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), drive_item);

			// submenu
			GtkWidget *submenu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(drive_item), submenu);

			// loop partitions
			for (size_t j = 0; j < drives[i].part_count; j++) {
				PartitionInfo *part = malloc(sizeof(PartitionInfo));
				*part = drives[i].parts[j];

				char *part_label = g_strdup_printf("%s %s [%s] %s", 
						part->mounted ? "Unmount" : "Mount",
						part->node,
						part->fstype,
						part->label);

				GtkWidget *part_item = gtk_image_menu_item_new_with_label(part_label);
				GtkWidget *part_image = gtk_image_new_from_icon_name(
						"device_usb", GTK_ICON_SIZE_MENU);
				gtk_image_menu_item_set_image(
						GTK_IMAGE_MENU_ITEM(part_item), part_image);
				g_signal_connect(part_item, "activate", 
						G_CALLBACK(on_partition_clicked), part);
				gtk_menu_shell_append(GTK_MENU_SHELL(submenu), part_item);
			}

			// power off item
			DriveCallbackData *cb = g_malloc(sizeof(DriveCallbackData));
			cb->node = drives[i].node;
			cb->model = drives[i].model;
			cb->parts = drives[i].parts;
			cb->part_count = drives[i].part_count;
			GtkWidget *power_off_item = gtk_menu_item_new_with_label("Power off");
			g_signal_connect(power_off_item, "activate", 
					G_CALLBACK(on_power_off_clicked), cb);
			gtk_menu_shell_append(GTK_MENU_SHELL(submenu), power_off_item);

		}
	}

   
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	// "Quit" item
	GtkWidget *quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(quit_item, "activate", G_CALLBACK(on_quit), NULL);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    gtk_widget_show_all(menu);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

static void on_left_click(GtkStatusIcon *status_icon, gpointer user_data) 
{
	show_menu(status_icon, 1, gtk_get_current_event_time());
}

int main(int argc, char *argv[]) 
{
    gtk_init(&argc, &argv);
	notify_init("mountatray");

    GtkStatusIcon *tray = gtk_status_icon_new_from_icon_name("drive-harddisk");
    gtk_status_icon_set_tooltip_text(tray, "mountatray");
    gtk_status_icon_set_visible(tray, TRUE);

    g_signal_connect(tray, "activate", G_CALLBACK(on_left_click), NULL);

    gtk_main();
	notify_uninit();

    return 0;
}

