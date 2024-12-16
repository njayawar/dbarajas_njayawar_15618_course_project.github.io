# Helper file to generate a series of randomized test vectors for all given ISCAS-85 benches

import pathlib
import random

if __name__ == "__main__":
    myTestDir = "./test_vectors/"
    myBenchDir = pathlib.Path("../resource/benchmarks/ISCAS85")
    myBenchFileType = ".bench"
    for myBenchFileHandle in myBenchDir.iterdir():
        myBenchInputs = list()
        if myBenchFileHandle.is_file() and myBenchFileHandle.suffix == myBenchFileType:
            with myBenchFileHandle.open('r') as myBenchFile:
                for myLine in myBenchFile:
                    if myLine.startswith("INPUT") or myLine.startswith("input"):
                        myLine = myLine.replace(")", " ")
                        myLine = myLine.strip()
                        myTokens = myLine.split('(')
                        myBenchInputs.append(myTokens[-1])

            myBenchName = str(myBenchFileHandle).split('/')[-1].split('.')[0]

            for myNumVectors in [1, 2, 4, 8, 16, 32, 64, 128, 256]:
                with open(myTestDir + myBenchName + "_" + str(myNumVectors) + ".test", "w") as myTestFile:
                    myTestFile.write("vectors " + str(myNumVectors) + "\n")

                    myTestFile.write("inputs " )
                    for myInput in myBenchInputs:
                        myTestFile.write(myInput + " ")
                    myTestFile.write("\n" )

                    for myCurrTestVector in range(myNumVectors):

                        myBinaryString = ""

                        for myCurrBit in range(len(myBenchInputs)):
                            if (random.randint(0, 1)):
                                myBinaryString = myBinaryString + "1"
                            else:
                                myBinaryString = myBinaryString + "0"

                        myTestFile.write(str(myCurrTestVector) + ": " + myBinaryString + "\n")

                    myTestFile.close()

            myBenchFile.close()
