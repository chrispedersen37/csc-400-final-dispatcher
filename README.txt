//Project for CSC-400

main(){
    input = recieveUserInput(); // recieves user input
    operation, filename, contents = inputHandler(input); // cleans the input and separates it into its specific parts (operation, filename and/or contents)
    if (readNumThreads()){ // detects if there is a thread available
        if(operation == save){
            pthread_create(saveFile(), filename, contents);
        }
        else if(operation == read){
            pthread_create(readFile(), filename);
        }
        else if(operation == delete){
            pthread_create(deleteFile(), filename);
        }
        else{
            print("unknown command");
        }
    }
    else{
        print("no threads available");
    }
}

saveFile(){

}

