#include <json-c/json.h>
#include <stdio.h>
#include <argp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


struct parser_args{
    json_object *root;
    int argc;
    char **argv;
    int *nargs;
    char *path;
    char *cfg_path;

};

int parser(int key,char *arg, struct argp_state *state){
    json_object *root = ((struct parser_args*)(state->input))->root;
    int argc=((struct parser_args*) (state->input))->argc;
    char **argv=((struct parser_args*)(state->input))->argv;
    int *nargs=((struct parser_args*)(state->input))->nargs;
    char *path=((struct parser_args*)(state->input))->path;

    switch(key){
        case 's':{ ;
            
            char *description=(char*) malloc(200);
            if(argc>=4 ){
                if(strlen(argv[3])>200){
                    printf("Description must not exceed 200 characters\n");
                    break;
                }
                strcpy(description,argv[3]);
            }
            else
                strcpy(description,"null");

            //Back up .nanorc
            char *cp=(char*) malloc(100);
            snprintf(cp,100,"%s%s%s","cp ~/.nanorc  ~/.nanocfgmgr/",arg,"\0" );
            system(cp);
            free(cp);
            //Check if entry exists and update description
            int i=0;
            int arr_len = json_object_array_length(root);
            while(i<arr_len){
                json_object *temp = json_object_array_get_idx(root, i);
                json_object *temp_name = json_object_object_get(temp,"name");
                if(strcmp(arg,json_object_get_string(temp_name))==0){
                    json_object *temp_desc=json_object_object_get(temp,"description");
                    json_object_set_string(temp_desc,description);
                    goto done;
                }
            }
            //Add new entry if needed
            json_object *entry=json_object_new_object();
            json_object_object_add(entry, "name", json_object_new_string(arg));
            json_object_object_add(entry, "description", json_object_new_string(description));
            json_object_array_add(root, entry);
            done: ;
            break;
                
        

        }
        //TODO: add cases for -l, -d , -p, --description
    }
    return 0;
}

int main(int argc, char **argv)
{
    
    int pathsize;
    if((pathsize=(pathconf("/",_PC_NAME_MAX)))==-1){
        if (errno == 0)
            puts("There is no limit to NAME_MAX.");
        else perror("pathconf() error");
        pathsize=255;
    }
    //Initialize file paths
    char *path=(char*) malloc(pathsize);
    snprintf(path, pathsize, "%s/.nanocfgmgr", getenv("HOME"));
    char *cfg_path=(char*) malloc(pathsize);
    snprintf(cfg_path,pathsize, "%s/.configs.json",path);  
    
    //Create default directory
    char *mkdir=(char*) malloc(100);
    snprintf(mkdir,100,"%s%s%s","mkdir ",path,"> /dev/null 2>&1" );
    system(mkdir);
    free(mkdir);
    json_object *root = json_object_from_file(cfg_path);
    //Open or create .json file
    if (!root){
      	FILE *f=fopen(cfg_path,"w");
        if(!f){
            printf("NO FILE\n");
        }
      	fclose(f);
       	root = json_object_from_file(cfg_path);
       	root=json_object_new_array();
    }
	
    struct argp_option options[]={
        {"save",'s',"NAME",0,"Backs up the configuration file Usage: nanocfgmr NAME [DESCRIPTION]"},
        {"load",'l',"NAME",0,"Loads the configuration file into .nanorc"},
        {"delete",'d',"NAME",0,"Deletes a saved configuration file"},
        {"print",'p',0,0,"Lists the saved files"},
        {"description",0,0,0,"Lists the saved files along with their descriptions"}, {0}
    };

    struct argp argp={options,parser};



	int nargs=0;

    struct parser_args *input= (struct parser_args*) malloc(sizeof(struct parser_args));
    input->root=root;
    input->argc=argc;
    input->argv=argv;
    input->nargs=&nargs;
    input->path=path;
    input->cfg_path=cfg_path;

    //TODO: add usage
    int success=argp_parse(&argp,argc,argv,0,0,input);
    json_object_to_file(cfg_path,root);
    json_object_put(root);
    return success;
}