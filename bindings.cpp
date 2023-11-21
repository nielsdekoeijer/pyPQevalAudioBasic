#include "PQevalAudio.h"
#include <AO.h>
#include <array>
#include <assert.h>
#include <exception>
#include <iostream>
#include <libtsp.h>
#include <libtsp/AFpar.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_SUCCESS */
#include <string>

#define ABSV(x) (((x) < 0) ? -(x) : (x))
#define ICEILV(n, m) (((n) + ((m)-1)) / (m)) /* int n,m >= 0 */
#define MAXV(a, b) (((a) > (b)) ? (a) : (b))

namespace py = pybind11;
double *getPtr(const py::array_t<double> &array) {
  py::buffer_info info = array.request();
  double *ptr;
  if (info.ndim == 1) {
    ptr = static_cast<double *>(info.ptr);
  } else {
    throw std::runtime_error("Array must be 1D (mono)");
  }

  for (std::size_t i = 0; i < array.size(); i++) {
    ptr[i] *= 32768.;
  }

  return ptr;
}

static struct PQ_MOVBC *PQ_InitMOVC(int Nchan, int Np);

void wavfile(py::array_t<double> ref, py::array_t<double> test,
             PQ_Opt options) {
  double *refPtr = getPtr(ref);
  double **xR = &refPtr;
  double *testPtr = getPtr(test);
  double **xT = &testPtr;
  int Fstart, Fend, Np, Nwup, is, i, j;
  int Nch;
  double DI, ODG;
  struct PQ_Par PQpar;
  struct PQ_FiltMem Fmem[2];
  struct PQ_MOVBI MOVBI[2];
  struct PQ_MOVBC *MOVC;
  double MOV[PQ_NMOV_MAX];

  Nch = 1;
  Fstart = 0;
  Fend = ((ref.size() + 1 - options.EndMin) / PQ_CB_ADV);
  Fend = MAXV(Fstart, Fend);

  /* Number of PEAQ frames */
  Np = Fend - Fstart + 1;
  if (options.Ni < 0)
    options.Ni = ICEILV(Np, ABSV(options.Ni));

  /* Initialize the MOV structure */
  MOVC = PQ_InitMOVC(Nch, Np);
  MOVC->PD.c1 = options.PDfactor;

  /* Fill in the parameter tables */
  PQgenTables(PQ_BASIC, &options, &PQpar);
  for (j = 0; j < Nch; ++j)
    PQinitFMem(PQpar.CB.Nc, options.PCinit, &Fmem[j]);

  is = 0;
  for (i = -Fstart; i < Np; ++i) {

    /* Read a frame of data */
    xR = xR + PQ_CB_ADV;
    xT = xT + PQ_CB_ADV;

    /* Process a frame */
    for (j = 0; j < Nch; ++j)
      MOVBI[j] = PQeval(xR[j], xT[j], &PQpar, &Fmem[j]);

    if (i >= 0) {
      /* Move the MOV precursors into a new structure */
      PQframeMOVB(MOVBI, Nch, i, MOVC);

      /* Print the MOV precursors */
      if (options.Ni != 0 && i % options.Ni == 0)
        PQprtMOVCi(Nch, i, MOVC);
    }
  }

  /* Time average of the MOV values */
  if (options.OverlapDelay)
    Nwup = Fstart;
  else
    Nwup = 0;

  PQavgMOVB(MOVC, Nch, Nwup, Np, MOV);

  /* Neural net */
  ODG = PQnNet(MOV, &PQpar.NNet, &DI);

  /* Summary printout */
  PQprtMOV(MOV, PQ_NMOV_B, DI, ODG);
}

static struct PQ_MOVBC *PQ_InitMOVC(int Nchan, int Np) {
  int Ni, NC, N, j;
  double *x;
  double *xp;
  static struct PQ_MOVBC MOVC;

  Ni = 2 * Np;
  NC = 11 * Np;
  if (Nchan == 1)
    N = Ni + NC;
  else
    N = Ni + 2 * NC;
  x = (double *)UTmalloc(N * sizeof(double));

  xp = x;
  MOVC.PD.Pc = xp;
  xp += Np;
  MOVC.PD.Qc = xp;
  xp += Np;

  for (j = 0; j < Nchan; ++j) {
    MOVC.Loud.NRef[j] = xp;
    xp += Np;
    MOVC.Loud.NTest[j] = xp;
    xp += Np;
    MOVC.MDiff.Mt1B[j] = xp;
    xp += Np;
    MOVC.MDiff.Mt2B[j] = xp;
    xp += Np;
    MOVC.MDiff.Wt[j] = xp;
    xp += Np;
    MOVC.NLoud.NL[j] = xp;
    xp += Np;
    MOVC.BW.BWRef[j] = xp;
    xp += Np;
    MOVC.BW.BWTest[j] = xp;
    xp += Np;
    MOVC.NMR.NMRavg[j] = xp;
    xp += Np;
    MOVC.NMR.NMRmax[j] = xp;
    xp += Np;
    MOVC.EHS.EHS[j] = xp;
    xp += Np;
  }

  assert(xp - x == N);

  return &MOVC;
}

void initDefault(PQ_Opt &self) { OPTIONS_INIT(&self); }

PYBIND11_MODULE(pyPQevalAudioBasic, m) {
  py::class_<PQ_Opt>(m, "BasicOptions")
      .def(py::init<>())
      .def("default", initDefault)
      .def_readwrite("Lp", &PQ_Opt::Lp)
      .def_readwrite("Ni", &PQ_Opt::Ni)
      .def_readwrite("ClipMOV", &PQ_Opt::ClipMOV)
      .def_readwrite("PCinit", &PQ_Opt::PCinit)
      .def_readwrite("PDfactor", &PQ_Opt::PDfactor)
      .def_readwrite("OverlapDelay", &PQ_Opt::OverlapDelay)
      .def_readwrite("DataBounds", &PQ_Opt::DataBounds)
      .def_readwrite("EndMin", &PQ_Opt::EndMin);

  m.def("wavfiles", &wavfile, "Run PEAQ-Basic on files");
}
