#ifndef __PROJECT1_CPP__
#define __PROJECT1_CPP__

#include "project1.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 4) // Checks that at least 3 arguments are given in command line
    {
        std::cerr << "Expected Usage:\n ./assemble infile1.asm infile2.asm ... infilek.asm staticmem_outfile.bin instructions_outfile.bin\n" << std::endl;
        exit(1);
    }
    //Prepare output files
    std::ofstream inst_outfile, static_outfile;
    static_outfile.open(argv[argc - 2], std::ios::binary);
    inst_outfile.open(argv[argc - 1], std::ios::binary);
    std::vector<std::string> instructions;

    /**
     * Phase 1:
     * Read all instructions, clean them of comments and whitespace DONE
     * TODO: Determine the numbers for all static memory labels
     * (measured in bytes starting at 0)
     * TODO: Determine the line numbers of all instruction line labels
     * (measured in instructions) starting at 0
    */
//    std::vector<std::string> staticVars;
//    std::unordered_map<std::string, int> staticMap;
    std::unordered_map<std::string, int> smLabels;
    std::unordered_map<std::string, int> instructionLabelsMap;
    std::vector<std::string> staticValue; // Vector to store all values from .word directives
    int smAddress = 0;                    // 0 is the first address for static memory
    int instructionLineCount = 0;         // counter for instruction lines
    bool staticMemoryFound = false;       // Will be true when we are in static memory
    //For each input file:
    for (int i = 1; i < argc - 2; i++) {
        std::ifstream infile(argv[i]); //  open the input file for reading
        if (!infile) { // if file can't be opened, need to let the user know
            std::cerr << "Error: could not open file: " << argv[i] << std::endl;
            exit(1);
        }

        // bool data = false; 
        std::string str;
        while (getline(infile, str)){ //Read a line from the file
            str = clean(str); // remove comments, leading and trailing whitespace
            if (str == "") { //Ignore empty lines
                continue;
            }

            // if(str == ".data"){
            //     data = true;
            //     continue;
            // }

            // if (data ==true){
            //     staticVars.push_back(str);
            // }

            // if(str== ".txt"){
            //     data = false;
            //     continue;
            // }

            // how to determine whether the current line is part of the instruction section or the static memory section?
            //Labels
            instructions.push_back(str); // TODO This will need to change for labels
            if (str[0] == '.')
            {
                continue; // Skip any line starts with "."
            }

            size_t wordPos = str.find(".word");
            if (wordPos != std::string::npos)
            {
                staticMemoryFound = true; // Switch to processing static memory when ".word" has been found
            }

            if (str[0] != '.' && wordPos == std::string::npos)
            {
                staticMemoryFound = false; // Switch to processing instruction
            }

            if (!staticMemoryFound)
            {
                // Process instruction labels and instructions
                size_t colonPos = str.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string label = str.substr(0, colonPos);
                    instructionLabelsMap[label] = instructionLineCount;
                    instructionLineCount--;
                }
                instructionLineCount++;
                continue;
            }
            else
            {
                // Processing static memory labels
                size_t colonPos = str.find(':');
                if (colonPos != std::string::npos)
                {
                    std::string label = str.substr(0, colonPos);
                    smLabels[label] = smAddress; // give static labels their respective offset
                    std::string valuesStr = str.substr(wordPos + 6);       // +6 to skip ":.word"
                    std::vector<std::string> values = split(valuesStr, " ");
                    smAddress += (values.size() * 4); 

                    // Check each value in values and append each value to the staticValue vector
                    for (const std::string &value : values)
                    {
                        staticValue.push_back(value);
                    }
                    continue;
                }
            }
            infile.close();
        }
        smLabels["_END_OF_STATIC_MEMORY_"] = smAddress;
        }
        //infile.close();
    

    /** Phase 2
     * Process all static memory, output to static memory file
     * TODO: All of this
     */

    // change to get a better static memory
    for (std::string values : staticValue)
    { // going through each value after .word
        std::vector<std::string> terms1 = split(values, WHITESPACE + ",()");
        auto foundValue = instructionLabelsMap.find(values); // could check if it is a label

        if (foundValue != instructionLabelsMap.end()) // if the label is found on the instruction_Labels map
        {                                       
            int write = foundValue->second * 4; // The next value occupies 4 bytes of memory
            write_binary(write, static_outfile);
        }

        else
        {                                  // Assume that it's an int
            int write = std::stoi(values); // convert the value as an integer
            write_binary(write, static_outfile);
        }
    }

    // int i = 0;
    // for(std::string stat:staticVars){
    //     std::vector<std::string> terms = split(stat, WHITESPACE+",()");
    //     std::string varName = terms[0];
    //     staticMap[varName] = i
    //     //Artur will have this but add terms[0] to the list of pointers
    //     std::string inst_type=terms[1];
    //     if (inst_type == ".int"){
    //         for(int y = 2, int y < size(terms), y++){
    //             int result = encode_static(terms[y]);
    //             write_binary(encode_static(terms[y]),inst_static_outfile);
    //             i=i+32;
    //     }}

    /** Phase 3
     * Process all instructions, output to instruction memory file
     * TODO: Almost all of this, it only works for adds
     */
    int count = 0;
    for(std::string inst : instructions) {
        std::vector<std::string> terms = split(inst, WHITESPACE+",()");
        std::string inst_type = terms[0];

        if (inst_type == "add") {
            int result = encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32);
            write_binary(encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32),inst_outfile);
        }
        
        // Code to write the addi instructions to memory in binary
        else if (inst_type == "addi")
        {
            std::cout << smLabels[terms[3]];
            int result = encode_Itype(8, registers[terms[2]], registers[terms[1]], std::stoi(terms[3]));
            write_binary(encode_Itype(8, registers[terms[2]], registers[terms[1]], std::stoi(terms[3])), inst_outfile);
        }

        // Code to write the sub instructions to memory in binary
        else if (inst_type == "sub")
        {
            int result = encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 34);
            write_binary(encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 34), inst_outfile);
        }

        // Code to write the mult instructions to memory in binary
        else if (inst_type == "mult")
        {
            int result = encode_Rtype(0, registers[terms[1]], registers[terms[2]], 0, 0, 24);
            write_binary(encode_Rtype(0, registers[terms[1]], registers[terms[2]], 0, 0, 24), inst_outfile);
        }

        // Code to write the div instructions to memory in binary
        else if (inst_type == "div")
        {
            int result = encode_Rtype(0, registers[terms[1]], registers[terms[2]], 0, 0, 26);
            write_binary(encode_Rtype(0, registers[terms[1]], registers[terms[2]], 0, 0, 26), inst_outfile);
        }

        // Code to write the mflo instructions to memory in binary
        else if (inst_type == "mflo")
        {
            int result = encode_Rtype(0, 0, 0, registers[terms[1]], 0, 18);
            write_binary(encode_Rtype(0, 0, 0, registers[terms[1]], 0, 18), inst_outfile);
        }

        // Code to write the mfhi instructions to memory in binary
        else if (inst_type == "mfhi")
        {
            int result = encode_Rtype(0, 0, 0, registers[terms[1]], 0, 16);
            write_binary(encode_Rtype(0, 0, 0, registers[terms[1]], 0, 16), inst_outfile);
        }

        // Code to write the sll instructions to memory in binary
        else if (inst_type == "sll")
        {
            int result = encode_Rtype(0, 0, registers[terms[2]], registers[terms[1]], std::stoi(terms[3]), 0);
            write_binary(encode_Rtype(0, 0, registers[terms[2]], registers[terms[1]], std::stoi(terms[3]), 0), inst_outfile);
        }

        // Code to write the srl instructions to memory in binary
        else if (inst_type == "srl")
        {
            int result = encode_Rtype(0, 0, registers[terms[2]], registers[terms[1]], std::stoi(terms[3]), 2);
            write_binary(encode_Rtype(0, 0, registers[terms[2]], registers[terms[1]], std::stoi(terms[3]), 2), inst_outfile);
        }

        // Code to write the lw instructions to memory in binary
        else if (inst_type == "lw")
        {
            int result = encode_Itype(35, registers[terms[3]], registers[terms[1]], std::stoi(terms[2]));
            write_binary(encode_Itype(35, registers[terms[3]], registers[terms[1]], std::stoi(terms[2])), inst_outfile);
        }

        // Code to write the sw instructions to memory in binary
        else if (inst_type == "sw")
        {
            int result = encode_Itype(43, registers[terms[3]], registers[terms[1]], std::stoi(terms[2]));
            write_binary(encode_Itype(43, registers[terms[3]], registers[terms[1]], std::stoi(terms[2])), inst_outfile);
        }

        // Code to write the slt instructions to memory in binary
        else if (inst_type == "slt")
        {
            int result = encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 42);
            write_binary(encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 42), inst_outfile);
        }

        // Code to write the bne instructions to memory in binary
        else if (inst_type == "bne")
        {
            // Term is adjusted in accordance with the formula on the guidelines sheet.
            std::string constant = (terms[3]);

            int term = findLabelLine(constant, instructionLabelsMap);
            term = term - (count + 1);
            int result = encode_Itype(5, registers[terms[1]], registers[terms[2]], term);
            write_binary(encode_Itype(5, registers[terms[1]], registers[terms[2]], term), inst_outfile);
        }

        // Code to write the beq instructions to memory in binary
        else if (inst_type == "beq")
        {
            // Term is adjusted in accordance with the formula on the guidelines sheet.
            std::string constant = (terms[3]);
            int term = findLabelLine(constant, instructionLabelsMap);
            term = term - (count + 1);
            // term = term * -1;
            // term = term - (count+1);

            int result = encode_Itype(4, registers[terms[1]], registers[terms[2]], term);
            write_binary(encode_Itype(4, registers[terms[1]], registers[terms[2]], term), inst_outfile);
        }

        // Code to write the j instructions to memory in binary
        else if (inst_type == "j")
        {
            std::string constant = (terms[1]);
            int term = findLabelLine(constant, instructionLabelsMap);
            int result = encode_Jtype(2, term);
            write_binary(encode_Jtype(2, term), inst_outfile);
        }

        // Code to write the jr instructions to memory in binary
        else if (inst_type == "jr")
        {
            int result = encode_Rtype(0, registers[terms[1]], 0, 0, 0, 8);
            write_binary(encode_Rtype(0, registers[terms[1]], 0, 0, 0, 8), inst_outfile);
        }

        // Code to write the jal instructions to memory in binary
        else if (inst_type == "jal")
        {
            std::string constant = (terms[1]);
            int term = findLabelLine(constant, instructionLabelsMap);

            int result = encode_Jtype(3, term);
            write_binary(encode_Jtype(3, term), inst_outfile);
        }

        // code to write the jalr instructions to memory in binary
        else if (inst_type == "jalr")
        {
            // check if only two terms (jalr $ra)
            if (terms.size() == 2)
            {
                int result = encode_Rtype(0, registers[terms[1]], 0, 31, 0, 9);
                write_binary(encode_Rtype(0, registers[terms[1]], 0, 31, 0, 9), inst_outfile);
            }
            else
            {
                int result = encode_Rtype(0, registers[terms[1]], 0, registers[terms[2]], 0, 9);
                write_binary(encode_Rtype(0, registers[terms[1]], 0, registers[terms[2]], 0, 9), inst_outfile);
            }
        }

        // Code to write the syscall instructions to memory in binary
        else if (inst_type == "syscall")
        {
            int result = encode_Rtype(0, 0, 0, 26, 0, 12);
            write_binary(encode_Rtype(0, 0, 0, 26, 0, 12), inst_outfile);
        }

        // Code to write the la instructions to memory in binary
        else if (inst_type == "la") {
            //for this project translate la $rt, label into  addi $rt, $zero, constant
            //constant is the address of the label in static memory (found in smLabels)
            
            // Assuming the instruction tokens (aka terms) are:
            // terms[0] = "la"
            // terms[1] = destination register (e.g., "$t0")
            // terms[2] = label (e.g., "myData")
            
            std::string rt = terms[1];      // destination register
            std::string label = terms[2];   // static memory label to load the address from
            
            // Look up the label's address from the static memory label map (smLabels)
            int constant = smLabels[label];
            
            // For addi, the opcode is 8 (decimal) and the source register is $zero
            int result = encode_Itype(8, registers["$zero"], registers[rt], constant);
            
            // Write the encoded addi instruction to the instruction output file
            write_binary(result, inst_outfile);
        }
        else{
            continue;
        }
        
        count++;
    }

}
#endif
