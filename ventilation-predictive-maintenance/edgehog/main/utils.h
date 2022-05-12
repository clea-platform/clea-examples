#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Convert a mac address to the char representation
 *
 * @param mac_addr An array of length 6 that contains tha 6 values that make the mac address.
 * @return char* The string representation.
 */
char *mac_addr2str(uint8_t *mac_addr)
{
    int length = snprintf(NULL, 0, "%x%x%x%x%x%x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    char *str = malloc(length + 1);
    snprintf(str, length + 1, "%x%x%x%x%x%x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return str;
}