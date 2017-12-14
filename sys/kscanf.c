//
// Created by Toby Babu on 11/19/17.
//
#include <sys/io.h>
#include <sys/defs.h>
#include <sys/allocator.h>
#include <sys/klibc.h>

const static char scantochar[100] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0',
                                     'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
                                     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x',
                                     'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' '};
const static char scantoUchar[100] = {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0',
                                      '\0',
                                      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0',
                                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X',
                                      'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' '};
const static char scantoCchar[100] = {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0',
                                      '\0',
                                      'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', '\0',
                                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X',
                                      'C', 'V', 'B', 'N', 'M', ',', '.', '/', '\0', '\0', '\0', ' '};
static int controlflag = 0x00;
static int size;
static char key_press[3];
static char *address_of_buf;
static char *terminal_buf;

/*void read_from_ip(int read_size, char* buf) {
    address_of_buf = buf;
    size = read_size;
    while (size >= 0) {
        if(size == 0) {
            return buf;
        }
    }
    return "";
}*/

void initialize_keyboard_buffer() {
    terminal_buf = bump(1024);
    address_of_buf = terminal_buf;
    size = 0;
}

int get_terminal_size() {
    int k=kstrlength(terminal_buf);
    return k;
}

void reset_terminal() {
    address_of_buf = terminal_buf;
    for (int i = 0; i < 1024; i++) {
        *(address_of_buf + i) = '\0';
    }
    size = 0;
}

char *get_terminal_buf() {
    return terminal_buf;
}

void kscanf(uint8_t keyscancode) {
    char key_character = '\0';
    //char key_press[3];

    if ((keyscancode & 0x80) == 0) {
        if (keyscancode == 14) {
            char *output_string = get_buffer_value();
            output_string -= 2;
            set_buffer_value(output_string);
            kprintf(" ");
            set_buffer_value(output_string);
            address_of_buf--;
            size--;
            *address_of_buf = '\0';
            address_of_buf--;
            size--;
        } else if (keyscancode == 58) //caps lock
        {
            if (controlflag & 0x01) {
                controlflag = controlflag & 0xfe;
            } else
                controlflag = controlflag | 0x01;
        } else if (keyscancode == 42 || keyscancode == 54) //shift
        {
            controlflag = controlflag | 0x02;
        } else if (keyscancode == 29) // control
        {
            controlflag = controlflag | 0x04;
        } else if (keyscancode == 28) //enter
        {
            key_character = scantochar[keyscancode];
            if (size >= 0 && address_of_buf != 0x0) {
                *address_of_buf = key_character;
                *(address_of_buf + 1) = '\0';
            }
            kprintf("\n");
        } else {

            if (controlflag & 0x04) //ctrl
            {
                key_character = '^';
                key_press[0] = key_character;
                kprintf("%c", key_character);
                if (size >= 0 && address_of_buf != 0x0) {
                    *address_of_buf = '^';
                    address_of_buf++;
                    //*(address_of_buf + 1) = '\0';
                }
                key_character = scantoCchar[keyscancode];
                if (size >= 0 && address_of_buf != 0x0) {
                    *address_of_buf = key_character;
                    *(address_of_buf + 1) = '\0';
                }
                key_press[1] = key_character;
                key_press[2] = '\0';
                controlflag = controlflag & 0xfb;
            } else if (controlflag & 0x02) //shift
            {
                key_character = scantoUchar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
                controlflag = controlflag & 0xfd;
            } else if (controlflag & 0x01) //caps lock
            {
                key_character = scantoUchar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
            } else {
                key_character = scantochar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
            }
            if (size >= 0 && address_of_buf != 0x0) {
                *address_of_buf = key_character;
                *(address_of_buf + 1) = '\0';
            }
            kprintf("%c", key_character);
        }

        if (keyscancode != 58 && keyscancode != 42 && keyscancode != 54 && keyscancode != 29) {
            if (size != 1024 && *address_of_buf != '\0') {
                address_of_buf++;
                size++;
            } else {
                reset_terminal();
            }
        }
    }

    //return key_press;
}




