#include <stdint.h>
#include <string.h>

void usb_init(void);
void usb_poll(void);
void usb_send(uint8_t ep, const void *data, uint16_t len);
void usb_recv(uint8_t ep, void (*cb)(const uint8_t *data, uint16_t len));
void rndis_init(void);
void rndis_poll(void);
void rndis_notify(uint16_t status);
void clock_init(void);
void ip_handle(const uint8_t *frame, uint16_t len);
void eth_handle_frame(const uint8_t *frame, uint16_t len);
void eth_send_frame(const uint8_t *frame, uint16_t len);
void arp_handle(const uint8_t *frame, uint16_t len);
const uint8_t *usb_get_device_descriptor(uint16_t *len);
const uint8_t *usb_get_config_descriptor(uint16_t *len);
void rndis_send_ethernet(const uint8_t *eth_frame, uint16_t len);

#define IO_BANK0_BASE 0x40014000
#define PADS_BANK0_BASE 0x4001C000
#define GPIO_CTRL_OFFSET 0x08
#define FUNCSEL_SIO 5
#define LED_PIN 25
#define SIO_BASE 0xD0000000
#define SIO_GPIO_OE_CLR (*(volatile uint32_t *)(SIO_BASE + 0x24))
#define SIO_GPIO_OE_SET (*(volatile uint32_t *)(SIO_BASE + 0x20))
#define SIO_GPIO_OUT_XOR (*(volatile uint32_t *)(SIO_BASE + 0x1C))
#define RNDIS_INITIALIZE_MSG 0x00000002
#define RNDIS_INITIALIZE_CMPLT 0x80000002
#define RNDIS_QUERY_MSG 0x00000004
#define RNDIS_QUERY_CMPLT 0x80000004
#define RNDIS_SET_MSG 0x00000005
#define RNDIS_SET_CMPLT 0x80000005
#define RNDIS_KEEPALIVE_MSG 0x00000008
#define RNDIS_KEEPALIVE_CMPLT 0x80000008
#define USBCTRL_BASE 0x50100000
#define RESETS_BASE 0x4000C000
#define CLOCKS_BASE 0x40008000
#define USB_SIE_CTRL (*(volatile uint32_t *)0x50110050)
#define USB_EP_CTRL(ep) (*(volatile uint32_t *)(0x50100080 + (ep)*4))
#define USB_BUFF_CTRL(ep) (*(volatile unsigned int *)(USBCTRL_BASE + 0x108 + (ep)*0x40))
#define XOSC_CTRL (*(volatile uint32_t *)(0x40024000))
#define PLL_SYS_CTRL (*(volatile uint32_t *)(0x40028000))
#define CLK_SYS_CTRL (*(volatile uint32_t *)(CLOCKS_BASE))
#define USB_SIE_STATUS (*(volatile uint32_t *)0x50110054)
#define USB_BUFF_STATUS (*(volatile uint32_t *)(0x50100054))
#define XOSC_STATUS (*(volatile uint32_t *)(0x40024004))
#define USB_IN_EP0_BUFFER ((volatile uint8_t *)(USBCTRL_BASE + 0x100))
#define USB_OUT_EP0_BUFFER ((volatile uint8_t *)(USBCTRL_BASE + 0x180))
#define USB_EP_IN_BUFFER(ep) ((volatile uint8_t *)(0x50100180 + (ep)*0x80))
#define USB_EP_OUT_BUFFER(ep) ((volatile uint8_t *)(0x50100100 + (ep)*0x80))
#define RESETS_RESET_DONE (*(volatile uint32_t *)(RESETS_BASE + 0x8))
#define CLK_USB_CTRL (*(volatile uint32_t *)(0x40008054))
#define RESETS_RESET (*(volatile uint32_t *)RESETS_BASE)
#define USBCTRL_REGS ((volatile uint32_t *)0x50110000)
#define USBCTRL_DPRAM ((volatile uint32_t *)USBCTRL_BASE)
#define PLL_SYS_PWR (*(volatile uint32_t *)(0x40028004))
#define PLL_SYS_FBDIV (*(volatile uint32_t *)(0x40028008))
#define WATCHDOG_CTRL (*(volatile uint32_t *)(0x40058000))

static uint8_t ctrl_resp[64];
static const uint8_t device_descriptor[] = {
  0x12,       // bLength
  0x01,       // bDescriptorType (Device)
  0x00, 0x02, // bcdUSB 2.00
  0xEF,       // bDeviceClass (Miscellaneous)
  0x02,       // bDeviceSubClass (Common Class)
  0x01,       // bDeviceProtocol (Interface Association Descriptor)
  0x40,       // bMaxPacketSize0
  0xC0, 0x16, // idVendor  = 0x16C0
  0xE1, 0x05, // idProduct = 0x05E1
  0x00, 0x01, // bcdDevice
  0x01,       // iManufacturer
  0x02,       // iProduct
  0x03,       // iSerialNumber
  0x01        // bNumConfigurations
};

static const uint8_t config_descriptor[] = {
  0x09, 0x02, 0x43, 0x00, 0x02, 0x01, 0x00, 0xC0, 0x32,
  0x08, 0x0B, 0x00, 0x02, 0xE0, 0x01, 0x03, 0x00,
  0x09, 0x04, 0x00, 0x00, 0x01, 0xE0, 0x01, 0x03, 0x00,
  0x05, 0x24, 0x00, 0x10, 0x01,
  0x05, 0x24, 0x01, 0x00, 0x01,
  0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0x0A,
  0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00, 0x00,
  0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00,
  0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00
};
static const uint8_t ms_os_string_descriptor[] = {
  0x12, 0x03,
  'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
  0x01, 0x00
};
static const uint8_t ms_compat_id_descriptor[] = {
  0x28, 0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 'R', 'N', 'D', 'I', 'S', 0, 0, 0,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint8_t mac[6] = { 0x02, 0x00, 0xde, 0xad, 0xbe, 0xef };
const uint8_t ip_addr[4] = { 192, 168, 7, 2 };

void disable_watchdog(void) {
  WATCHDOG_CTRL = 0;
}

static inline void delay(volatile int d) {
  while (d--)
    ;
}

static inline void gpio25_init(void) {
  volatile uint32_t *gpio25_ctrl = (uint32_t *)(IO_BANK0_BASE + 25 * GPIO_CTRL_OFFSET);
  *gpio25_ctrl = (*gpio25_ctrl & ~0x1F) | FUNCSEL_SIO;
  volatile uint32_t *pad25 = (uint32_t *)(PADS_BANK0_BASE + 25 * 4);
  *pad25 = (*pad25 & ~((1 << 3) | (1 << 2))) | (1 << 7);
}

void setup() {
  disable_watchdog();
  clock_init();
  SIO_GPIO_OE_SET = (1u << LED_PIN);
  for (int i = 0; i < 5; i++) {
    SIO_GPIO_OUT_XOR = (1u << LED_PIN);
    delay(200000);
  }
  usb_init();
  rndis_init();
}

void loop() {
  usb_poll();
  rndis_poll();
}

void clock_init(void) {
  volatile uint32_t *PLL_USB_CTRL = (uint32_t *)(0x4002C000);
  volatile uint32_t *PLL_USB_PWR = (uint32_t *)(0x4002C004);
  volatile uint32_t *PLL_USB_FBDIV = (uint32_t *)(0x4002C008);
  *PLL_USB_PWR = (1 << 5);
  *PLL_USB_FBDIV = 40;
  *PLL_USB_CTRL = (5 << 12) | (2 << 8) | (1 << 0);
  *PLL_USB_PWR = 0;
  while (!(*PLL_USB_PWR & (1 << 31)))
    ;
}

void ip_handle(const uint8_t *f, uint16_t l) {
  uint8_t ihl = (f[14] & 0x0F) * 4;
  uint8_t proto = f[23];
  if (proto == 1) {
    const uint8_t *icmp = f + 14 + ihl;
    if (icmp[0] == 8) {
      uint16_t totlen = (f[16] << 8) | f[17];
      uint8_t resp[64];
      memset(resp, 0, 64);
      memcpy(resp + 6, f + 6, 6);
      memcpy(resp, f + 6, 6);
      memcpy(resp + 14, f + 12, 2);
      memcpy(resp + 12, f + 12, 2);
      resp[23] = 1;
      resp[20] = 0;
      memcpy(resp + 14 + ihl, icmp, totlen - ihl - 14);
      eth_send_frame(resp, totlen + 14);
    }
  }
}

void eth_handle_frame(const uint8_t *f, uint16_t l) {
  if (memcmp(f, mac, 6) == 0 || memcmp(f, "\xff\xff\xff\xff\xff\xff", 6) == 0) {
    uint16_t type = (f[12] << 8) | f[13];
    if (type == 0x0806) arp_handle(f, l);
    else if (type == 0x0800) ip_handle(f, l);
  }
}

void eth_send_frame(const uint8_t *f, uint16_t l) {
  uint8_t buf[64];
  memcpy(buf, f, l);
  usb_send(1, buf, l + 44);
}

const uint8_t *usb_get_device_descriptor(uint16_t *len) {
  *len = sizeof(device_descriptor);
  return device_descriptor;
}

const uint8_t *usb_get_config_descriptor(uint16_t *len) {
  *len = sizeof(config_descriptor);
  return config_descriptor;
}

void handle_ctrl(const uint8_t *d, uint16_t l) {
  uint32_t t = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
  memset(ctrl_resp, 0, 64);
  switch (t) {
    case RNDIS_INITIALIZE_MSG:
      ctrl_resp[0] = 0x02;
      ctrl_resp[1] = 0x00;
      ctrl_resp[2] = 0x00;
      ctrl_resp[3] = 0x80;
      ctrl_resp[4] = 0x1C;
      ctrl_resp[8] = 0x01;
      ctrl_resp[12] = 0x00;
      ctrl_resp[16] = 0x00;
      ctrl_resp[17] = 0x40;
      memcpy(ctrl_resp + 4, d + 4, 4);
      break;
    case RNDIS_QUERY_MSG:
      ctrl_resp[0] = 0x04;
      ctrl_resp[1] = 0x00;
      ctrl_resp[2] = 0x00;
      ctrl_resp[3] = 0x80;
      ctrl_resp[4] = 0x14;
      ctrl_resp[8] = 0x00;
      ctrl_resp[12] = 0x14;
      ctrl_resp[16] = 0x06;
      memcpy(ctrl_resp + 20, mac, 6);
      break;
    case RNDIS_SET_MSG:
      ctrl_resp[0] = 0x05;
      ctrl_resp[1] = 0x00;
      ctrl_resp[2] = 0x00;
      ctrl_resp[3] = 0x80;
      ctrl_resp[4] = 0x10;
      ctrl_resp[8] = 0x00;
      break;
    case RNDIS_KEEPALIVE_MSG:
      ctrl_resp[0] = 0x08;
      ctrl_resp[1] = 0x00;
      ctrl_resp[2] = 0x00;
      ctrl_resp[3] = 0x80;
      ctrl_resp[4] = 0x10;
      ctrl_resp[8] = 0x00;
      break;
    default:
      return;
  }
}

void handle_data(const uint8_t *d, uint16_t l) {
  uint8_t *payload = (uint8_t *)(d + 44);
  uint32_t plen = l - 44;
  eth_handle_frame(payload, plen);
}

void rndis_init(void) {
  usb_recv(0, handle_ctrl);
  usb_recv(1, handle_data);
}

void rndis_poll(void) {}

void arp_handle(const uint8_t *f, uint16_t l) {
  if (l < 42) return;
  const uint8_t *sha = f + 6;
  const uint8_t *spa = f + 28;
  const uint8_t *tpa = f + 38;
  if (memcmp(tpa, ip_addr, 4) == 0) {
    uint8_t resp[42];
    memset(resp, 0, 42);
    memcpy(resp, sha, 6);
    memcpy(resp + 6, mac, 6);
    resp[12] = 0x08;
    resp[13] = 0x06;
    resp[20] = 0x00;
    resp[21] = 0x02;
    resp[22] = 0x08;
    resp[23] = 0x00;
    memcpy(resp + 24, mac, 6);
    memcpy(resp + 28, ip_addr, 4);
    memcpy(resp + 30, sha, 6);
    memcpy(resp + 34, spa, 4);
    eth_send_frame(resp, 42);
  }
}

static void (*ep_out_handler[16])(const uint8_t *, uint16_t);

void usb_init(void) {
  // Configurar DPRAM para EP0, EP1 y EP2
  USBCTRL_DPRAM[0] = 0x00000000;  // EP0 OUT
  USBCTRL_DPRAM[1] = 0x00000040;  // EP0 IN
  USBCTRL_DPRAM[2] = 0x00000080;  // EP1 OUT
  USBCTRL_DPRAM[3] = 0x000000C0;  // EP1 IN
  USBCTRL_DPRAM[4] = 0x00000100;  // EP2 IN (interrupt)

  // Configurar EP1 como Bulk IN/OUT (0x02)
  USB_EP_CTRL(1) = (0x40 << 16) | (0x02 << 8) | 0x01;         // IN
  USB_EP_CTRL(1) |= (0x40 << 16) | (0x02 << 8) | (0x01 << 16); // OUT

  // Configurar EP2 como Interrupt IN (0x03)
  USB_EP_CTRL(2) = (0x08 << 16) | (0x03 << 8) | 0x02;

  // Habilitar pull-up y controlador USB
  USB_SIE_CTRL = (1 << 24) | (1 << 0); // ENABLE | PULLUP_EN

  // Limpiar estado
  USB_SIE_STATUS = 0xFFFFFFFF;
}

void usb_poll(void) {
  if (USB_SIE_STATUS & (1 << 10)) {  // SETUP_RECEIVED
    uint8_t setup[8];
    for (int i = 0; i < 8; i++) {
      setup[i] = USB_OUT_EP0_BUFFER[i];
    }

    uint8_t bmRequestType = setup[0];
    uint8_t bRequest = setup[1];
    uint16_t wValue = (setup[3] << 8) | setup[2];
    uint16_t wIndex = (setup[5] << 8) | setup[4];
    uint16_t wLength = (setup[7] << 8) | setup[6];

    if ((bmRequestType & 0x60) == 0x00) {  // Standard Request
      if (bRequest == 6) { // GET_DESCRIPTOR
        const uint8_t *d = NULL;
        uint16_t len = 0;

        if ((wValue >> 8) == 1) {
          d = usb_get_device_descriptor(&len);
        } else if ((wValue >> 8) == 2) {
          d = usb_get_config_descriptor(&len);
        } else if ((wValue >> 8) == 3 && (wValue & 0xFF) == 0xEE) {
          d = ms_os_string_descriptor;
          len = sizeof(ms_os_string_descriptor);
        }

        if (d != NULL) {
          if (len > wLength) len = wLength;
          memcpy((void *)USB_IN_EP0_BUFFER, d, len);
          USB_BUFF_CTRL(0) = (len << 16) | 0x80000000;
        }
      }
    } else if ((bmRequestType & 0x60) == 0x20) {  // Class Request
      if (bmRequestType == 0x21 && bRequest == 0x00) {  // SEND_ENCAPSULATED_COMMAND
        handle_ctrl((const uint8_t *)(USB_OUT_EP0_BUFFER + 8), wLength);
        USB_BUFF_CTRL(0) = 0x80000000; // ZLP para status stage
      } else if (bmRequestType == 0xA1 && bRequest == 0x01) {  // GET_ENCAPSULATED_RESPONSE
        memcpy((void *)USB_IN_EP0_BUFFER, ctrl_resp, 64);
        USB_BUFF_CTRL(0) = (64 << 16) | 0x80000000;
      }
    } else if ((bmRequestType == 0xC0) && (bRequest == 0x01) && (wIndex == 0x0004)) {
      memcpy((void *)USB_IN_EP0_BUFFER, ms_compat_id_descriptor, sizeof(ms_compat_id_descriptor));
      USB_BUFF_CTRL(0) = (sizeof(ms_compat_id_descriptor) << 16) | 0x80000000;
    }

    USB_SIE_STATUS = (1 << 10);  // Clear SETUP_RECEIVED
  }

  // Manejo de datos OUT en endpoints
  uint32_t buff_status = USB_BUFF_STATUS;
  for (int ep = 1; ep < 16; ep++) {
    if (buff_status & (1 << (ep * 2))) {
      if (ep_out_handler[ep]) {
        uint8_t buf[64];
        for (int i = 0; i < 64; i++) {
          buf[i] = USB_EP_OUT_BUFFER(ep)[i];
        }
        ep_out_handler[ep](buf, 64);
      }
      USB_BUFF_STATUS = (1 << (ep * 2)); // Clear flag
    }
  }
}

void usb_send(uint8_t ep, const void *data, uint16_t len) {
  const uint8_t *src = (const uint8_t *)data;
  for (int i = 0; i < len; i++) {
    USB_EP_IN_BUFFER(ep)
    [i] = src[i];
  }
  USB_BUFF_STATUS = (1 << (ep * 2));
}

void usb_recv(uint8_t ep, void (*cb)(const uint8_t *data, uint16_t len)) {
  ep_out_handler[ep] = cb;
}

void rndis_send_ethernet(const uint8_t *eth_frame, uint16_t len) {
  uint8_t buf[512];
  memset(buf, 0, sizeof(buf));
  buf[0] = 0x01;
  buf[4] = (len + 44) & 0xFF;
  buf[5] = (len + 44) >> 8;
  buf[8] = 0x2C;
  buf[12] = len & 0xFF;
  buf[13] = len >> 8;
  memcpy(buf + 44, eth_frame, len);
  usb_send(1, buf, len + 44);
}

void rndis_notify(uint16_t status) {
  uint8_t buf[8] = { 0 };
  usb_send(2, buf, 8);
}
