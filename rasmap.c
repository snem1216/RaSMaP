/*
        RaSMaP: Rise and Shine Magic Packet
        A Wake-on-Lan console menu for Linux systems.
        Copyright (C) 2019 Steven Nemeti <https://github.com/snem1216>
        This software is released under the MIT license
         and is provided with no warranty whatsoever.
        Dependencies: etherwake, ncurses
*/
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#define NUMLETTERS 26

// Dict
typedef struct {
    char comp_name[16];
    char mac_address[18];
} comp_list_t;

// Get the appropriate respective start locations for specified text
int centeredlocation(char text[])  
{
        struct winsize window_size;
        ioctl(0, TIOCGWINSZ, &window_size);
        return (window_size.ws_col / 2) - (strlen(text) / 2);
}
int rightjustifiedlocation(char text[])  
{
        struct winsize window_size;
        ioctl(0, TIOCGWINSZ, &window_size);
        return window_size.ws_col - strlen(text);
}

// Puts the status in the bottom bar, on the left.
int setstatus(char *status_text)
{
        attron(A_REVERSE);
        struct winsize window_size;
        ioctl(0, TIOCGWINSZ, &window_size);
        
        // Blank-out the status bar.
        //  This ensures that a previous, longer message won't display at the end
        //   of the one that we are about to set. 
        char blank[window_size.ws_col];

        // The - 15 here is so it doesn't overwrite "Press q to exit." in the bottom-right.
        // This will likely be improved later on.
        sprintf(blank,"%%-%ds",window_size.ws_col - 15);
        mvprintw(window_size.ws_row - 1,0,blank, " ");
        mvprintw(window_size.ws_row - 1,1,status_text);
        attroff(A_REVERSE);
        return 0;
}

// Send magic packet to the specified MAC address
int wake_up(char mac_address[])
 // Grab a brush and put a little makeup
{
        //  Hide command output
        char *command;
        asprintf(&command, "etherwake %s > /dev/null", mac_address);
        // Return error code of etherwake.
        return system(command);
}

// Display menu and wait for user input on loop until they press q.
int display_menu(comp_list_t *comp_list, const int num_options)
{
        // Get window dimensions for reference
        struct winsize window_size;
        ioctl(0, TIOCGWINSZ, &window_size);

        // Menu's on-screen position
        int x_offset = 2;
        int y_offset = 1;
        
        // Vertical padding relative to window
        int v_padding = 2;

        // Selected menu option.
        // May change this later to remember the last selected computer.
        int selected_index = 0;

        // Maximum on-screen height of the menu, and scroll offset
        // If the menu height is too large for the number of options available
        //  on the menu, then make it as large as the number of options.
        // For some reason an if/else statement wouldn't work,
        //  so this was the next best option.
        int menu_height = window_size.ws_row - (v_padding * 2);
        if (menu_height > num_options)
        {
                menu_height = num_options;
        }
        int scroll_offset = 0;

        // 15 characters is recommended, as it is the max length of a hostname & IP address.
        int entry_width = 15;

        // Get the cursor out of view.
        curs_set(0);

        if (selected_index > menu_height)
        {
                scroll_offset = selected_index-menu_height + 1;
        }

        // Create template
        char blank[entry_width];
        // Pad the string of each entry so they are equal in size.
        sprintf(blank, "[ %%-%ds ]", entry_width);

        // Menu loop
        int pressed_key = -1;
        // While pressed key is not q
        while(pressed_key != 113)
        {
                // Print all options to screen
                int options_index;
                for (options_index=0; options_index < menu_height; options_index++)
                {
                        // If this is the selected option
                        if (options_index+scroll_offset==selected_index)
                        {
                                // Highlight it
                                attron(A_REVERSE);
                        }
                        mvprintw(x_offset+options_index,y_offset,blank,comp_list[options_index+scroll_offset].comp_name);
                        attroff(A_REVERSE);
                }

                // Get input
                pressed_key=getch();
                switch(pressed_key)
                {
                        // Enter / Return
                        case 10:
                                if(wake_up(comp_list[selected_index].mac_address) == 0)
                                {
                                        char *status;
                                        asprintf(&status, "Sent magic packet to %s",comp_list[selected_index].comp_name);
                                        setstatus(status);
                                }
                                break;
                        // Goes to top / bottom of list, respectively.
                        case KEY_HOME:
                                selected_index=0;
                                scroll_offset=0;
                                break;
                        case KEY_END:
                                selected_index=num_options-1;
                                scroll_offset=num_options-menu_height;
                                break;

                        // Single up & down
                        case KEY_UP:
                                if (selected_index)
                                {
                                        selected_index--;
                                        if (selected_index < scroll_offset)
                                        {
                                                scroll_offset--;
                                        }
                                }
                                break;
                        case KEY_DOWN:
                                if (selected_index < num_options-1)
                                {
                                        selected_index++;
                                        if (selected_index > scroll_offset+menu_height-1)
                                        {
                                                scroll_offset++;
                                        }     
                                }
                                break;
                }
        }
        return 0;
}
int main(void)
{
        // Determine if running as root, to prevent etherwake errors.
        if(geteuid() != 0)
        {
                printf("ERROR: This program must be run as root.\n");
                return 3;
        }
        
        #pragma region Get Config

        // Window display elements.
        //  01-08-2019: Still in alpha until an official Linux package is released.
        char windowtitle[] = "RaSMaP v0.91-alpha";
        char copyright[] = "(C) 2019 Steven Nemeti (MIT License)";
        // If you change this, make sure to change the status bar clearing offset
        //  in set_status! 
        char exit_notif[] = "Press q to exit.";

        // Rough list, will include commented lines.
        comp_list_t temp_list[NUMLETTERS];
        // Final list that the menu will use.
        comp_list_t comp_list[NUMLETTERS];
        
        // Configuration file
        FILE *conf_file;
        char conf_file_path[] = "rasmap.conf";
        conf_file = fopen(conf_file_path, "r");
        if (conf_file == NULL) {
                fprintf(stderr, "ERROR: Could not read/find config file: %s\nMake sure the file exists and you have read permissions.\n",conf_file_path);
                return 1;
        }
        size_t count = 0;
        while (fscanf(conf_file, "%s = %s", &temp_list[count].comp_name, &temp_list[count].mac_address) == 2) {
                count++;
        }

        // comp_list index
        int cli = 0;

        // Remove commented lines that were added during the initial parse
        //  This is so the menu can use the array "as is" without skipping indexes of 
        //  parsed commented lines
        // temp_list index
        int tli;
        for(tli = 0; tli < count; tli++)
        {
                if(temp_list[tli].comp_name[0] != '#')
                {
                comp_list[cli] = temp_list[tli];
                cli++;
                }
        }
        #pragma endregion
        
        #pragma region Window Preparation

        // Initialize screens
        initscr();
        noecho();
        keypad(stdscr,TRUE);

        // Get Window dimensions
        struct winsize window_size;
        ioctl(0, TIOCGWINSZ, &window_size);

        // Reverse foreground and background colors
        attron(A_REVERSE);

        // Blank title bar & status bar
        char blank[window_size.ws_col];
        sprintf(blank,"%%-%ds",window_size.ws_col);
        mvprintw(0,0,blank, " ");
        mvprintw(window_size.ws_row - 1,0,blank, " ");

        // Print window title & copyright on the menu bar
        // If the window is too small for the title to be justified center, justify it to left instead.
        if( (centeredlocation(windowtitle) + strlen(windowtitle)) >= rightjustifiedlocation(copyright) )
        {
                mvprintw(0,0,windowtitle);
        }
        else
        {
                mvprintw(0,centeredlocation(windowtitle),windowtitle);
        }
        mvprintw(0,rightjustifiedlocation(copyright),copyright);
        mvprintw(window_size.ws_row - 1,rightjustifiedlocation(exit_notif),exit_notif);

        #pragma endregion
        
        // Toggle color reversal off
        attroff(A_REVERSE);
        display_menu(comp_list,cli);
        refresh();
        endwin();
        return 0;
}
