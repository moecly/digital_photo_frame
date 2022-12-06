#ifndef _COMMON_H
#define _COMMON_H

#ifndef NULL
#define NULL (void *)0
#endif

typedef struct region {
  int left_up_x;
  int left_up_y;
  int width;
  int height;
} region;

typedef struct region_cartesian {
  int left_up_x;
  int left_up_y;
  int width;
  int height;
} region_cartesian;

#define for_each_linked_node(root, node)                                              \
  for (node = root; node != NULL; node = node->next)

#define for_each_array_node(root, node) for (node = root; node != NULL; node++)

#endif // !_COMMON_H
