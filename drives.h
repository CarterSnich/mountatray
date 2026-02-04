#ifndef DRIVES_H
#define DRIVES_H

#include <stddef.h>

typedef struct {
    char *node;
    char *fstype;
	char *label;
	int mounted;
} PartitionInfo;

typedef struct {
    char *node;
    char *model;
	PartitionInfo *parts;
	size_t part_count;
} DriveInfo;

DriveInfo *get_external_drives(size_t *count);
void free_external_drives(DriveInfo *drives, size_t count);
void mount_partition(const char *node);
void unmount_partition(const char *node);
void power_off_drive (const char *node);

#endif

