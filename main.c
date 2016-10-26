#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "command.h"
#include "main.h"

/*
 * How to add additional commands:
 *  1. Include your file:
 *          #include "yourfile.h"
 *  2. In main() add your command with createCommand():
 *          createCommand("cmd_name",&cmd_method);
 */

struct command_node *last;

/**
 * Purpose:
 *  End user input loop.
 */
void quit()
{
    stop = true;
}

/**
 * Purpose:
 *  Print all of the commands.
 */
void help()
{
    printf("Commands:\n");
    struct command_node *cmd = commands;
    do
    {
        printf("\t%s\n", cmd->name);
    } while ((cmd = cmd->next));
}

/**
 * Purpose:
 *  Add a command for the minix terminal.
 * @params:
 *  char *name: the name of the command.
 *  commandFunction method: the method to execute when the command is run.
 */
void createCommand(char *name, commandFunction method)
{
    struct command_node *new_cmd = (struct command_node *) malloc(sizeof(struct command_node));
    new_cmd->name = name;
    new_cmd->run = method;
    new_cmd->cmdLen = strlen(name);

    if (!commands)
    {
        commands = new_cmd;
        last = commands;
    }
    else
    {
        last->next = new_cmd;
        last = last->next;
    }
}

// Use a standard way rather than write one because standards are good.
// http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace((unsigned char) *str))
    {
        str++;
    }

    if (*str == 0)
    {  // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char) *end))
    {
        end--;
    }

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

/**
 * Purpose:
 *  1. Read user input.
 *  2. Parse command.
 *  3. TODO: Parse args.
 *  4. Find and run command.
 */
void prompt()
{
    char buf[MAX_INPUT_SIZE];
    write(STD_OUTPUT, PROMPT_SYMBOL, strlen(PROMPT_SYMBOL));
    int rawRead = read(STD_INPUT, buf, MAX_INPUT_SIZE);

    // Trim spaces that could cause errors.
    char *trimmed = trimwhitespace(buf);
    int left = strlen(trimmed);

    char *args[left];

    // If there are no spaces, the command
    // is the same as the trimmed command.
    char *cmdStr = trimmed;
    args[0] = trimmed;
    int wordsFound = 1;
    for (int i = 0; i < left; i++)
    {
        if (trimmed[i] == ' ')
        {
            trimmed[i] = 0;
            args[wordsFound] = &trimmed[i+1];
            wordsFound++;
        }
    }
    //sometimes this number is wrong according to debug.
    //not sure how yet but believe this is the source
    //of your flag problem. (at least one possible source)	
    int cmdLen = strlen(args[0]);

    if (DEBUG)
    {
        for (int i = 0; i < wordsFound; i++)
        {
            printf("\t----%d:%s\n",i,args[i]);
        }
    }

    bool found = false;
    for (struct command_node *cmd = commands; !found && cmd; cmd = cmd->next)
    {
        if (DEBUG)
        {	
            printf("\t----%d=%d\n", cmd->cmdLen, cmdLen);
            printf("\t----'%s'='%s'\n", cmd->name, cmdStr);
        }

        /* This will short circuit if the lengths
         * do not match. Ints are more efficient
         * to compare than strings so compare size
         * before doing strcmp.
         * If the strings are same strcmp returns 0.
         */
        if (cmd->cmdLen == cmdLen && !strcmp(cmd->name, cmdStr))
        {
            if (DEBUG)
            {
                printf("\t----RUN COMMAND\n");
                cmd->run(wordsFound, args);		
                found = true;                
		break;
            	
	    }
	}
    }

    if (!found)
    {
        printf("%s: command not found\n", cmdStr);
    }

    // There seems to be a problem with data being
    // left in buf if 6+ chars were read. This messes
    // up the next command. Clearing the buffer seems
    // to fix this.
    for (int i = 0; i < rawRead; i++)
    {
        buf[i] = 0;
    }
}
// mount selected file, throw error if file is invalid.
// map superblock attributes to variables in 'command.h'
// this probably has to do with error in the console
// but sometimes this method will work and sometimew it
// won't using the same input

void mount(int numArgs,char *args[]){

	char *imagefile;
	
	if (numArgs > 1){
        	imagefile = args[1];
    	}
	
	else{
        	return;
	}

	int fd = open(imagefile, O_RDONLY);
	
	if(fd == -1){
		printf("Invalid Input, file wasn't mounted\n");
	}
	
	else{		
		lseek(fd, 1024, SEEK_SET);
		read(fd, &super , 20);
		
		printf("Mounted: %s\n", imagefile);
	}
}
void showsuper(){
	
	printf("\n\tnumber of inodes: %d", super.s_ninodes);
	printf("\n\tnumber of zones: %d", super.s_nzones);	
	printf("\n\tnumber of imap_blocks: %d", super.s_imap_blocks); 	      
        printf("\n\tnumber of zmap_blocks: %d", super.s_zmap_blocks);	
	printf("\n\tfirst data zone: %d", super.s_firstdatazone); 	
	printf("\n\tlog zone size: %d", super.s_log_zone_size);		
	printf("\n\tmax size: %d", super.s_max_size); 		
	printf("\n\tmagic: %d", super.s_magic); 			
	printf("\n\tstate: %d", super.s_state); 			
	printf("\n\tzones: %d\n", super.s_zones);			
}
int main()
{
    createCommand("quit", &quit);
    createCommand("help", &help);
    createCommand("mount", &mount);
    createCommand("showsuper",&showsuper);
    do
    {
        prompt();
    } while (!stop);

    printf("\n");
    return 0;
}
