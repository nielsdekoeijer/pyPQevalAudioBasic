from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import subprocess
import sys
import pybind11

class CustomBuildExtCommand(build_ext):
    """Custom build command."""

    def run(self):
        # Call your Makefile here
        retcode = subprocess.call(['make', 'clean', '-C', 'AFsp-v9r0'])
        if retcode != 0:
            sys.exit(retcode)
        retcode = subprocess.call(['make', 'COPTS="-fPIC"', '-C', 'AFsp-v9r0'])
        if retcode != 0:
            sys.exit(retcode)
        super().run()

# Define the extension module
ext_modules = [
    Extension(
        'pyPQevalAudioBasic',
        ['bindings.cpp'],  # Replace with your source files using pybind11
        include_dirs=[
            pybind11.get_include(),
            './AFsp-v9r0/include/',
            './AFsp-v9r0/audio/PQevalAudio/'
        ],
        language='c++',
        extra_compile_args=['-O3', '-fPIC'],
        extra_link_args=['-s', '-lm'],
        extra_objects=[
            './AFsp-v9r0/audio/PQevalAudio/PQevalAudio.o',
            './AFsp-v9r0/audio/PQevalAudio/PQeval.o',
            './AFsp-v9r0/audio/PQevalAudio/PQnNet.o',
            './AFsp-v9r0/audio/PQevalAudio/CB/PQgroupCB.o',
            './AFsp-v9r0/audio/PQevalAudio/CB/PQspreadCB.o',
            './AFsp-v9r0/audio/PQevalAudio/CB/PQtimeSpread.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQdataBoundary.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQgenTables.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQinitFmem.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQoptions.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQreadChan.o',
            './AFsp-v9r0/audio/PQevalAudio/Misc/PQtableFn.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQframeMOV.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQloudTest.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovBW.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovEHS.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovModDiffB.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovNLoudB.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovNMRB.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQmovPD.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQprtMOV.o',
            './AFsp-v9r0/audio/PQevalAudio/MOV/PQprtMOVCi.o',
            './AFsp-v9r0/audio/PQevalAudio/Patt/PQadapt.o',
            './AFsp-v9r0/audio/PQevalAudio/Patt/PQmodPatt.o', 
            './AFsp-v9r0/audio/PQevalAudio/Patt/PQloud.o',
            './AFsp-v9r0/lib/libAO.a',
            './AFsp-v9r0/lib/libtsplite.a',
        ],

    ),
]

setup(
    name='pyPQevalAudioBasic',
    version='0.1',
    ext_modules=ext_modules,
    cmdclass={
        'build_ext': CustomBuildExtCommand,
    },
    # ... other setup.py parameters ...
)
