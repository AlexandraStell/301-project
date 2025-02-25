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

    //For each input file:
    for (int i = 1; i < argc - 2; i++) {
        std::ifstream infile(argv[i]); //  open the input file for reading
        if (!infile) { // if file can't be opened, need to let the user know
            std::cerr << "Error: could not open file: " << argv[i] << std::endl;
            exit(1);
        }

        std::string str;
        while (getline(infile, str)){ //Read a line from the file
            str = clean(str); // remove comments, leading and trailing whitespace
            if (str == "") { //Ignore empty lines
                continue;
            }
            //labels
            instructions.push_back(str); // TODO This will need to change for labels
        }
        infile.close();
    }

    /** Phase 2
     * Process all static memory, output to static memory file
     * TODO: All of this
     */

    /** Phase 3
     * Process all instructions, output to instruction memory file
     * TODO: Almost all of this, it only works for adds
     */
    for(std::string inst : instructions) {
        std::vector<std::string> terms = split(inst, WHITESPACE+",()");
        std::string inst_type = terms[0];
        if (inst_type == "add") {
            int result = encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32);
            write_binary(encode_Rtype(0,registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 32),inst_outfile);
        }
        
        // Code to write the slt instructions to memory in binary
        if (inst_type == "slt")
        {
            int result = encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 42);
            write_binary(encode_Rtype(0, registers[terms[2]], registers[terms[3]], registers[terms[1]], 0, 42), inst_outfile);
        }

        // Code to write the bne instructions to memory in binary
        if (inst_type == "bne")
        {
            // Term is adjusted in accordance with the formula on the guidelines sheet.
            std::string constant = (terms[3]);

            int term = findLabelLine(constant, instructionLabelsMap);
            term = term - (count + 1);
            int result = encode_Itype(5, registers[terms[1]], registers[terms[2]], term);
            write_binary(encode_Itype(5, registers[terms[1]], registers[terms[2]], term), inst_outfile);
        }

        // Code to write the beq instructions to memory in binary
        if (inst_type == "beq")
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
        if (inst_type == "j")
        {
            std::string constant = (terms[1]);
            int term = findLabelLine(constant, instructionLabelsMap);
            int result = encode_Jtype(2, term);
            write_binary(encode_Jtype(2, term), inst_outfile);
        }

        // Code to write the jr instructions to memory in binary
        if (inst_type == "jr")
        {
            int result = encode_Rtype(0, registers[terms[1]], 0, 0, 0, 8);
            write_binary(encode_Rtype(0, registers[terms[1]], 0, 0, 0, 8), inst_outfile);
        }

        // Code to write the jal instructions to memory in binary
        if (inst_type == "jal")
        {
            std::string constant = (terms[1]);
            int term = findLabelLine(constant, instructionLabelsMap);

            int result = encode_Jtype(3, term);
            write_binary(encode_Jtype(3, term), inst_outfile);
        }

        // code to write the jalr instructions to memory in binary
        if (inst_type == "jalr")
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
        if (inst_type == "syscall")
        {
            int result = encode_Rtype(0, 0, 0, 26, 0, 12);
            write_binary(encode_Rtype(0, 0, 0, 26, 0, 12), inst_outfile);
        }
    }
}

#endif
