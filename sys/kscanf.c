//
// Created by Toby Babu on 11/19/17.
//
#include <sys/io.h>
#include <sys/defs.h>

const static char scantochar[100] = { '\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
                                     'q','w','e','r','t','y','u','i','o','p','\0','\0','\0','\0',
                                     'a','s','d','f','g','h','j','k','l','\0','\0','\0','\0','\0','z','x',
                                     'c','v','b','n','m' };
const static char scantoUchar[100] = { '\0','\0','!','@','#','$','%','^','&','*','(',')','\0','\0','\0','\0',
                                      'Q','W','E','R','T','Y','U','I','O','P','\0','\0','\0','\0',
                                      'A','S','D','F','G','H','J','K','L','\0','\0','\0','\0','\0','Z','X',
                                      'C','V','B','N','M' };
const static char scantoCchar[100] = { '\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
                                      'Q','W','E','R','T','Y','U','I','O','P','\0','\0','\0','\0',
                                      'A','S','D','F','G','H','J','K','L','\0','\0','\0','\0','\0','Z','X',
                                      'C','V','B','N','M' };
static int controlflag = 0x00;
static int size = -1;
static char key_press[3];
static char *address_of_buf = 0x0;

char* read_from_ip(int read_size, char* buf) {
    //size = read_size;
    address_of_buf = buf;
    size = 1;
    while (size >= 0) {
        //kprintf("here");
        if(size == 0) {
            //kprintf("gone");
            //size = -1;
            //address_of_buf = 0x0;
            return "test\0";
        }
        //size--;
    }
    return "";
}

void kscanf(uint8_t keyscancode) {
    char key_character = '\0';
    //char key_press[3];

    if ((keyscancode & 0x80) == 0)
    {
        if(keyscancode == 14) {
            char *output_string = get_buffer_value();
            output_string-=2;
            set_buffer_value(output_string);
            kprintf(" ");
            set_buffer_value(output_string);
        }
        else if (keyscancode == 58) //caps lock
        {
            if (controlflag & 0x01)
            {
                controlflag=controlflag & 0xfe;
            }
            else
                controlflag=controlflag | 0x01;
        }
        else
        if (keyscancode == 42 || keyscancode == 54) //shift
        {
            controlflag = controlflag | 0x02;
        }
        else
        if (keyscancode == 29) // control
        {
            controlflag = controlflag | 0x04;
        }
        else {

            if (controlflag & 0x04) //ctrl
            {
                key_character = '^';
                key_press[0] = key_character;
                kprintf("%c",key_character);
                if (size >= 0 && address_of_buf != 0x0){
                    *address_of_buf = '^';
                    address_of_buf++;
                    //*(address_of_buf + 1) = '\0';
                }
                key_character = scantoCchar[keyscancode];
                if (size >= 0 && address_of_buf != 0x0){
                    *address_of_buf = key_character;
                    *(address_of_buf + 1) = '\0';
                }
                key_press[1] = key_character;
                key_press[2] = '\0';
                controlflag = controlflag & 0xfb;
            }
            else
            if (controlflag & 0x02) //shift
            {
                key_character = scantoUchar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
                controlflag = controlflag & 0xfd;
            }
            else
            if (controlflag & 0x01) //caps lock
            {
                key_character = scantoUchar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
            }
            else {
                key_character = scantochar[keyscancode];
                key_press[0] = key_character;
                key_press[1] = '\0';
            }
                if (size >= 0 && address_of_buf != 0x0){
                    *address_of_buf = key_character;
                    *(address_of_buf + 1) = '\0';
                }
                kprintf("%c",key_character);
        }

    }
    if (size != -1) {
        address_of_buf++;
        size--;
    }
    //return key_press;
}
