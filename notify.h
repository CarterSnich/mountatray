#ifndef NOTIFY_HELPER_H
#define NOTIFY_HELPER_H

NotifyNotification *send_notification(const char *title, const char *body, const char *icon);
void update_notification(NotifyNotification *n, const char *title, 
		const char *body, const char *icon);

#endif
