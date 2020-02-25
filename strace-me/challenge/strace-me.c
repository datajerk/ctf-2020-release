#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>

// These are all the syscalls I can find that don't crash, don't hang, and print their first arg as hex
uint8_t SYSCALLS[] = { 5, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 21, 22, 25, 26, 31, 32, 33, 38, 39, 40, 42, 43, 45, 51, 52, 58, 59, 61, 74, 75, 76, 77, 78, 79, 82, 83, 84, 85, 86, 87, 88, 90, 91, 92, 96, 97, 98, 99, 101, 104, 105, 106, 107, 109, 113, 115, 116, 121, 122, 124, 125, 126, 127, 128, 129, 130, 134, 135, 137, 144, 149, 150, 151, 159, 160, 162, 163, 165, 166, 167, 168, 169, 171, 172, 175, 176, 177, 179, 182, 183, 184, 185, 186, 188, 189, 191, 192, 193, 195, 196, 198, 209, 211, 212, 217, 218, 219, 222, 223, 226, 227, 229, 230, 232, 233, 235, 236, 240, 243, 244, 246, 247, 248, 249, 251 };

// This is just the same boring code over and over - just calling a syscall with the first parameter changing
char *TEMPLATES[] = {
  "\xb8\x00\x00\x00\x00\xbb\x2e\x13\x01\x16\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x0a\x30\x3d\x21\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x32\x3b\x3a\x39\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x21\x0a\x27\x30\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x33\x0a\x30\x3d\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x0a\x32\x34\x39\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x21\x0a\x26\x3c\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x38\x0a\x30\x3d\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x0a\x30\x27\x3a\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x30\x21\x3b\x3c\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x21\x26\x30\x27\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x0a\x32\x3b\x3c\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x25\x21\x20\x3a\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x2c\x0a\x21\x20\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x34\x0a\x20\x3a\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x26\x0a\x30\x27\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x3b\x3c\x30\x30\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x3a\x26\x0a\x32\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x30\x3d\x21\x0a\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x3b\x3a\x39\x0a\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x39\x33\x0a\x32\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x3c\x0a\x32\x34\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x3a\x39\x0a\x26\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
  "\xb8\x00\x00\x00\x00\xbb\x28\x74\x32\x3b\xb9\x80\x80\x80\x80\xba\x80\x80\x80\x80\xbe\x80\x80\x80\x80\xbf\x80\x80\x80\x80\xcd\x80",
};

int main(int argc, char *argv[]){
  int syscall_count = sizeof(SYSCALLS) / sizeof(uint8_t);
  int template_count = sizeof(TEMPLATES) / sizeof(char*);
  int i;

  // Allocate a buffer for everything
  uint8_t *buffer = mmap(0, template_count * 32 + 7, PROT_EXEC |PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

  // Append the templates to the buffer, randomizing their syscall
  for(i = 0; i < template_count; i++) {
    // Copy into buffer
    memcpy(buffer + (32 * i), TEMPLATES[i], 32);

    // Randomize the syscall
    *(buffer + 32 * i + 1) = SYSCALLS[rand() % syscall_count];

    // Convert to big endian for simplicity but also to make this more confusing :)
    buffer[32 * i + 6] ^= 0x55;
    buffer[32 * i + 7] ^= 0x55;
    buffer[32 * i + 8] ^= 0x55;
    buffer[32 * i + 9] ^= 0x55;
  }
  memcpy(buffer + 32 * i, "\x31\xc0\x40\x31\xdb\xcd\x80", 7);

  asm("call *%0\n" : :"r"(buffer));
}