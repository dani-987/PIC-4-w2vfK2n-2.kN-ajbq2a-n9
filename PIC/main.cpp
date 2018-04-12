#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	/*
	Compiler c;
	while (1) {
		ASM* test = c.compileFile("D:\\Daniel\\Documents\\Visual Studio 2017\\Projects\\PIC\\PIC-4-w2vfK2n-2.kN-ajbq2a-n9\\Debug\\Testprogramme\\TPicSim1.LST", 200);
		ASM_CODE* tmp = nullptr;
		if (test != nullptr) {
			tmp = test->code;
		}
		else printf("Last Kompiler error: %s\n", c.getCompilerError());
		while (tmp != nullptr && tmp->function != nullptr) {
			printf("__asm('%s') 0x%x, 0x%x\n", Compiler::functionPointerToName(tmp->function), tmp->param1, tmp->param2);
			tmp++;
		}
		if(test != nullptr)c.freeASM(test);
		else { printf("test == nullptr\n"); system("pause"); }
		system("pause");
	}
	system("pause");
	/*/
	GUI* gui = new GUI();
	return gui->run();//*/
}