#include "../../include/raw_mode.h"
#include "../../include/input.h"
#include "../../include/output.h"

#define TOTAL 9
#define SEPRATOR printf("---------------------------------\n");
extern struct editorConfig E;

int test_new_line(){
    insert_new_line();
    if(E.cy==1 && E.cx==0){
        printf("New line test passed\n");
        return 1;
    }
    printf("New line test failed\n");
    return 0;
}
int default_mode_test(){
    if(E.mode==0){
        printf("Default mode is command mode...Test passed\n");
        return 1;
    }
    printf("Default mode is not command mode...Test failed\n");
    return 0;
}
int insert_mode_test(){
    process_keypress('i');
    if(E.mode==1){
        printf("Insert mode...Test passed\n");
        return 1;
    }
    printf("Not in insert mode...Test Failed\n");
    return 0;
}
int switch_to_default_mode(){
    process_keypress(ESC);
    if(E.mode==0){
        printf("Back to default mode...Test passed\n");
        return 1;
    }
    printf("Still in insert mode...Test Failed\n");
    return 0;
}
int insert_characters_test(){
    E.mode = 1;
    process_keypress('a');
    process_keypress('b');
    erow *row = &E.row[E.cy];
    if(strcmp(row->chars,"ab")==0){
        printf("Inserted characters successfully...Test passed\n");
        return 1;
    }
    printf("Insert characters failed...Test Failed\n");
    return 0;
}
int insert_characters_worng_mode(){
    insert_new_line();
    process_keypress('a');
    erow *row = &E.row[E.cy];
    if(strcmp(row->chars,"")==0){
        printf("Characters not inseted in command mode...Test passed\n");
        return 1;
    }
    printf("Characters inserted in command mode %s...Test failed\n",row->chars);
    return 0;
}
int home_key_test(){
    E.mode = 1;
    process_keypress('a');
    process_keypress(HOME_KEY);
    if(E.cx==0){
        printf("Home key test passed\n");
        return 1;
    }
    printf("Home key test failed\n");
    return 0;
}
int erase_test(){
    process_keypress(END_KEY);
    int rows = E.numrows;
    for(int i=0;i<rows;i++){
        for(int j=0;j<E.row[i].size+1;j++){
            process_keypress(BACKSPACE);
        }
    }
    process_keypress(BACKSPACE);
    if(E.cx==0 && E.cy==0){
        printf("Erased file content successfully...Test passed\n");
        return 1;
    }
    printf("Total line %d, %d, %d",E.numrows,E.cx,E.cy);
    printf("Could not erase file...Test Failed\n");
    return 0;
}
int test_save(){
    E.filename = "temp.c";
    process_keypress('a');
    process_keypress('b');
    process_keypress('c');
    save();
    char content[10];
    FILE *file;
    file = fopen("temp.c","r");
    fgets(content,4,file);
    fclose(file);
    if(strcmp(content,"abc")==0){
        printf("Saved file content successfully...Test passed\n");
        return 1;
    }
    printf("Could not save to the file...Test failed\n");
    return 0;
}
void print_test_results(int number_of_tests_passed,int total_test){
    printf("Test result: %d/%d tests passed\n",number_of_tests_passed,total_test);
}
int main(){
    int number_of_tests_passed = 0;

    SEPRATOR
    number_of_tests_passed += test_new_line();
    SEPRATOR
    number_of_tests_passed += default_mode_test();
    SEPRATOR
    number_of_tests_passed += insert_mode_test();
    SEPRATOR
    number_of_tests_passed += insert_characters_test();
    SEPRATOR
    number_of_tests_passed += switch_to_default_mode();
    SEPRATOR
    number_of_tests_passed += insert_characters_worng_mode();
    SEPRATOR
    number_of_tests_passed += home_key_test();
    SEPRATOR
    number_of_tests_passed += erase_test();
    SEPRATOR
    number_of_tests_passed += test_save();
    SEPRATOR

    SEPRATOR
    print_test_results(number_of_tests_passed,TOTAL);
    SEPRATOR
    SEPRATOR

}
