#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "list.h"

//   PID PR CPU% S  #THR     VSS     RSS PCY UID      Name
enum {
    PID, PR, CPU,S, THR, VSS, RSS, PCY, UID, NAME,
};

struct ps_record {
    int pid;
    int pr;
    int cpu;
    char s;
    int thr;
    int vss;
    int rss;
    char pcy[16];
    char uid[32];
    char name[256];
    struct listnode list;
};

/* global */
list_declare(ps_record_list);
char filter[256];

/* read a file, make sure it is terminated with \n\0 */
static char* read_file(const char* path, size_t* sz)
{
    char* data = NULL;
    int fd;
    ssize_t size = 0;

    fd = open(path, O_RDONLY);
    if (fd < 0) goto oops;

    size = lseek(fd, 0, SEEK_END);
    if (size <= 0) goto oops;

    if (lseek(fd, 0, SEEK_SET) < 0)
        goto oops;

    data = (char*)malloc(size + 2);
    
    if (read(fd, data, size) != size)
        goto oops;
    
    data[size] = '\n';
    data[size+1] = '\0';

    if (sz)
        *sz = size;

    return data;

oops:
    perror("read_file() error");
    close(fd);
    if (data) free(data);
    return NULL;
}

static void ps_record_print(struct ps_record *item)
{
    if (!item)
        return;

    fprintf(stderr, "%5d %2d %3d%% %c %5d %7dK %6dK %s %s %s\n",
        item->pid
        , item->pr
        , item->cpu
        , item->s
        , item->thr
        , item->vss
        , item->rss
        , item->pcy
        , item->uid
        , item->name);
}

static void ps_record_setat(struct ps_record *item, int index, char *name)
{
    switch(index) {
        case PID:
            item->pid = atoi(name);
        break;
        case PR:
            item->pr = atoi(name);
        break;
        case CPU:
            item->cpu = atoi(name);
        break;
        case S:
            item->s = (*name) & 0xFF;
        break;
        case THR:
            item->thr = atoi(name);
        break;
        case VSS:
            item->vss = atoi(name);
        break;
        case RSS:
            item->rss = atoi(name);
        break;
        case PCY:
            strcpy(item->pcy, name);
        break;
        case UID:
            strcpy(item->uid, name);
        break;
        case NAME:
            strcpy(item->name, name);
        break;
    }

    return;
}

static void _parse_top(const char* line, const char* filter)
{
    char* ptr;
    int flen;
    int i;
    struct ps_record *pItem;

    if (filter)
        flen = strlen(filter);
    
    if (!strstr(line, filter))
        return;
    
    pItem = (struct ps_record *)malloc(sizeof(*pItem));

    for (i = 0, ptr = (char*)line; ptr && *ptr; i++) {
        char *x;
        
        // remove space
        while(isspace(*ptr)) ptr++;
        
        x = strchr(ptr, ' ');
        if (x != NULL)
            *x++ = 0;
        
        ps_record_setat(pItem, i, ptr);
        
        ptr = x;
    }

    //ps_record_print(pItem);

    list_add_head(&ps_record_list, &(pItem->list));
}

static void parse_top(const char* data, const char* filter)
{
    char *ptr;

    ptr = (char*)data;
    while(ptr && *ptr) {
        char *x = strchr(ptr, '\n');
        if (x != NULL)
            *x++ = 0;
        _parse_top(ptr, filter);
        ptr = x;
    }
}

static void cal_ps_records_list()
{
    struct listnode *node, *n;
    int cpu, vss, rss;
    int nr;

    nr = 0;
    cpu = 0;
    vss = 0;
    rss = 0;

    list_for_each_safe(node, n, &ps_record_list) {
        struct ps_record* item = node_to_item(node, struct ps_record, list);

        nr++;
        cpu += item->cpu;
        vss += item->vss;
        rss += item->rss;

        list_del(&item->list);
        free(item);
    }

    fprintf(stderr,"AVG(%s): TOTAL COUNT = %-d CPU = %d%% VSS = %7dK, RSS=%6dK\n", 
        filter, nr, cpu / nr, vss / nr, rss / nr);
}

static void process_top(const char* path, const char* filter)
{
    char *data;
    size_t sz;

    data = read_file(path, &sz);

    if (data) {
        parse_top(data, filter);
        cal_ps_records_list();
        free(data);
    }

    return;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        char* name = strrchr(argv[0], '/') == NULL 
                        ? argv[0] : strrchr(argv[0], '/') + 1;
        fprintf(stderr, "Usage: %s <filename> <ps name>\n", name);
        fprintf(stderr, "   ex: %s android-top.log com.my.app\n", name);
        exit(1);
    }

    strcpy(filter, argv[2]);
    process_top(argv[1], argv[2]);

    return 0;
}