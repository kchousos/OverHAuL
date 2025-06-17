#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "libsniff.h"

// Provide a public wrapper to call the static function from libsniff.c
// We'll declare it here and define it in this translation unit.

int libsniff_open_raw_public(char *iface);

int libsniff_open_raw_public(char *iface) {
  // Forward call to the internal static function in libsniff.c
  // Just forward the call here, assuming we compile this harness
  // linking libsniff.c compiled with "static int libsniff_open_raw(char *)"
  // This means we need to move libsniff_open_raw definition to non-static
  // or define the function here literally. But since in provided source
  // it's static, we can define an identical function here for fuzzing.

  // Copy of libsniff_open_raw from libsniff.c (non-static)
  if (iface == NULL)
    return -1;
  struct ifreq ifr;
  struct ifreq ifr2;
  struct iwreq wrq;
  struct iwreq wrq2;
  struct packet_mreq mr;
  struct sockaddr_ll sll;
  struct sockaddr_ll sll2;
  int fd;

  if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    // perror("socket(PF_PACKET) failed");
    return -1;
  }

  if (strlen(iface) >= sizeof(ifr.ifr_name)) {
    // printf("Interface name too long: %s\n", iface);
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
  ifr.ifr_name[sizeof(ifr.ifr_name) - 1] = 0;
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    // fprintf(stderr, "Interface %s: \n", iface);
    // perror("ioctl(SIOCGIFINDEX) failed");
    close(fd);
    return -1;
  }
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = ifr.ifr_ifindex;
  sll.sll_protocol = htons(ETH_P_ALL);

  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    // fprintf(stderr, "Interface %s: \n", iface);
    // perror("ioctl(SIOCGIFHWADDR) failed");
    close(fd);
    return -1;
  }
  if ((ifr.ifr_flags | IFF_UP | IFF_BROADCAST | IFF_RUNNING) != ifr.ifr_flags) {
    ifr.ifr_flags |= IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
      // perror("ioctl(SIOCSIFFLAGS) failed");
      close(fd);
      return -1;
    }
  }
  if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
    // fprintf(stderr, "Interface %s: \n", iface);
    // perror("bind(ETH_P_ALL) failed");
    close(fd);
    return -1;
  }
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    // fprintf(stderr, "Interface %s: \n", iface);
    // perror("ioctl(SIOCGIFHWADDR) failed");
    close(fd);
    return -1;
  }
  if (ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211 &&
      ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_PRISM &&
      ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_FULL) {
    // fprintf(stderr,
    //        "\nUnsupported hardware link type %4d "
    //        "- expected ARPHRD_IEEE80211, ARPHRD_IEEE80211_FULL or ARPHRD_IEEE80211_PRISM instead.\n",
    //        ifr.ifr_hwaddr.sa_family);
    close(fd);
    return -1;
  }
  memset(&mr, 0, sizeof(mr));
  mr.mr_ifindex = sll.sll_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
    // perror("setsockopt(PACKET_MR_PROMISC) failed");
    close(fd);
    return -1;
  }
  // Normally fd should be returned, but for fuzzing just close it to prevent descriptor exhaustion
  close(fd);
  return 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0)
    return 0;

  // Prepare a buffer for interface name, ensure null termination
  // Interface name is limited max by sizeof(ifr.ifr_name) (usually IFNAMSIZ 16)
  // so cap to 64 bytes as safe upper bound here

  size_t bufsize = size < 64 ? size : 64;
  char iface[65];

  if (bufsize >= sizeof(iface))
    bufsize = sizeof(iface) - 1;

  memcpy(iface, data, bufsize);
  iface[bufsize] = 0;

  // Call the fuzzed function
  libsniff_open_raw_public(iface);

  return 0;
}