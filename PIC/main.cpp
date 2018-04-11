#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	while (1) {
		ASM* test = compileFile("D:\\Daniel\\Documents\\Visual Studio 2017\\Projects\\PIC\\PIC-4-w2vfK2n-2.kN-ajbq2a-n9\\Debug\\Testprogramme\\TPicSim1.LST", 200);
		ASM_CODE* tmp = nullptr;
		if (test != nullptr) {
			tmp = test->code;
		}
		else printf("Last Kompiler error: %s\n", getCompilerError());
		while (tmp != nullptr && tmp->function != nullptr) {
			printf("__asm('%s') 0x%x, 0x%x\n", functionPointerToName(tmp->function), tmp->param1, tmp->param2);
			tmp++;
		}
		if(test != nullptr)freeASM(test);
		else { printf("test == nullptr\n"); system("pause"); }
		system("pause");
	}
	system("pause");
	/*/
	GUI* gui = new GUI();
	return gui->run();//*/
}