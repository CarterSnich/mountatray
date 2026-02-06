#include <libnotify/notify.h>

NotifyNotification *send_notification(const char *title, const char *body, const char *icon) 
{
	NotifyNotification *n;
	n = notify_notification_new(title, body, icon);
	notify_notification_show(n, NULL);
	return n;
}

void update_notification(NotifyNotification *n, const char *title, 
		const char *body, const char *icon)
{
    if (!n)
        return;

    notify_notification_update(n, title, body, icon);
    notify_notification_show(n, NULL);
}

