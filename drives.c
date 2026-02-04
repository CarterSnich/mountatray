#include "drives.h"
#include <libudev.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static int is_mounted(const char *node)
{
    FILE *f = fopen("/proc/self/mounts", "r");
    if (!f) return 0;

    char dev[256], mp[256], fs[64];
    int mounted = 0;

    while (fscanf(f, "%255s %255s %63s %*s %*d %*d\n",
                  dev, mp, fs) == 3) {
        if (strcmp(dev, node) == 0) {
            mounted = 1;
            break;
        }
    }

    fclose(f);
    return mounted;
}

static void add_partition(struct udev *udev,
                          struct udev_device *disk,
                          DriveInfo *drive)
{
    struct udev_enumerate *e = udev_enumerate_new(udev);
    udev_enumerate_add_match_parent(e, disk);
    udev_enumerate_add_match_property(e, "DEVTYPE", "partition");
    udev_enumerate_scan_devices(e);

    struct udev_list_entry *entry;
    struct udev_list_entry *list = udev_enumerate_get_list_entry(e);

    udev_list_entry_foreach(entry, list) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *p = udev_device_new_from_syspath(udev, path);

        const char *node = udev_device_get_devnode(p);
        const char *fstype = udev_device_get_property_value(p, "ID_FS_TYPE");
		const char *label = udev_device_get_property_value(p, "ID_FS_LABEL");

        if (!node || !fstype) {
            udev_device_unref(p);
            continue;
        }

        drive->parts = realloc(drive->parts,
                               sizeof(PartitionInfo) * (drive->part_count + 1));

        drive->parts[drive->part_count].node = strdup(node);
        drive->parts[drive->part_count].fstype = strdup(fstype);
		drive->parts[drive->part_count].label = label ? strdup(label) : strdup("");
		drive->parts[drive->part_count].mounted = is_mounted(node);

        drive->part_count++;

        udev_device_unref(p);
    }

    udev_enumerate_unref(e);
}

DriveInfo *get_external_drives(size_t *count)
{
    *count = 0;
    DriveInfo *list = NULL;

    struct udev *udev = udev_new();
    if (!udev) return NULL;

    struct udev_enumerate *e = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(e, "block");
    udev_enumerate_scan_devices(e);

    struct udev_list_entry *entry;
    struct udev_list_entry *devices =
        udev_enumerate_get_list_entry(e);

    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev =
            udev_device_new_from_syspath(udev, path);

        const char *devtype = udev_device_get_devtype(dev);
        if (!devtype || strcmp(devtype, "disk") != 0)
            goto next;

        struct udev_device *parent =
            udev_device_get_parent_with_subsystem_devtype(
                dev, "usb", "usb_device");

        if (!parent)
            goto next;

        const char *node = udev_device_get_devnode(dev);
        if (!node || strstr(node, "loop"))
            goto next;

        const char *model =
            udev_device_get_property_value(dev, "ID_MODEL");

        DriveInfo d = {0};
        d.node = strdup(node);
        d.model = model ? strdup(model) : strdup("Unknown");

        add_partition(udev, dev, &d);

        list = realloc(list, sizeof(DriveInfo) * (*count + 1));
        list[*count] = d;
        (*count)++;

    next:
        udev_device_unref(dev);
    }

    udev_enumerate_unref(e);
    udev_unref(udev);
    return list;
}

void free_external_drives(DriveInfo *drives, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        free(drives[i].node);
        free(drives[i].model);

        for (size_t j = 0; j < drives[i].part_count; j++) {
            free(drives[i].parts[j].node);
            free(drives[i].parts[j].fstype);
        }

        free(drives[i].parts);
    }

    free(drives);
}

void mount_partition(const char *node)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "udisksctl mount -b %s", node);

    int ret = system(cmd);
    if (ret == 0) {
        printf("%s mounted successfully\n", node);
    } else {
        printf("Failed to mount %s\n", node);
    }
}

void unmount_partition(const char *node)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "udisksctl unmount -b %s", node);

    int ret = system(cmd);
    if (ret == 0) {
        printf("%s unmounted successfully\n", node);
    } else {
        printf("Failed to unmount %s\n", node);
    }
}

void power_off_drive(const char *node)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "udisksctl power-off -b %s", node);

    int ret = system(cmd);
    if (ret == 0) {
        printf("%s powered off successfully\n", node);
    } else {
        printf("Failed to power off %s\n", node);
    }
}

