#include "libsniff.h"

static int libsniff_open_raw(char *iface) {
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
    perror("socket(PF_PACKET) failed");
    return -1;
  }

  if (strlen(iface) >= sizeof(ifr.ifr_name)) {
    printf("Interface name too long: %s\n", iface);
    return -1;
  }
  /* find the interface index */
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, iface, sizeof(ifr.ifr_name) - 1);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    fprintf(stderr, "Interface %s: \n", iface);
    perror("ioctl(SIOCGIFINDEX) failed");
    return -1;
  }
  memset(&sll, 0, sizeof(sll));
  sll.sll_family = AF_PACKET;
  sll.sll_ifindex = ifr.ifr_ifindex;
  sll.sll_protocol = htons(ETH_P_ALL);

  /* lookup the hardware type */
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf(stderr, "Interface %s: \n", iface);
    perror("ioctl(SIOCGIFHWADDR) failed");
    return -1;
  }
  /* Is interface st to up, broadcast & running ? */
  if ((ifr.ifr_flags | IFF_UP | IFF_BROADCAST | IFF_RUNNING) != ifr.ifr_flags) {
    /* Bring interface up*/
    ifr.ifr_flags |= IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
      perror("ioctl(SIOCSIFFLAGS) failed");
      return -1;
    }
  }
  /* bind the raw socket to the interface */
  if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) //-V641
  {
    fprintf(stderr, "Interface %s: \n", iface);
    perror("bind(ETH_P_ALL) failed");
    return -1;
  }
  /* lookup the hardware type */
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf(stderr, "Interface %s: \n", iface);
    perror("ioctl(SIOCGIFHWADDR) failed");
    return -1;
  }
  if (ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211 &&
      ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_PRISM &&
      ifr.ifr_hwaddr.sa_family != ARPHRD_IEEE80211_FULL) {
    if (ifr.ifr_hwaddr.sa_family == ARPHRD_ETHERNET)
      fprintf(stderr, "\nARP linktype is set to 1 (Ethernet) ");
    else
      fprintf(stderr, "\nUnsupported hardware link type %4d ",
              ifr.ifr_hwaddr.sa_family);
    fprintf(stderr,
            "- expected ARPHRD_IEEE80211,\nARPHRD_IEEE80211_"
            "FULL or ARPHRD_IEEE80211_PRISM instead.  Make\n"
            "sure RFMON is enabled: run 'airmon-ng start %s"
            " <#>'\nSysfs injection support was not found "
            "either.\n\n",
            iface);
    return -1;
  }
  /* enable promiscuous mode */
  memset(&mr, 0, sizeof(mr));
  mr.mr_ifindex = sll.sll_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
    perror("setsockopt(PACKET_MR_PROMISC) failed");
    return -1;
  }
  return fd;
}
