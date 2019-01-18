/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2019 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file lp_data/HighsLp.h
 * @brief 
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#ifndef LP_DATA_HIGHS_LP_H_
#define LP_DATA_HIGHS_LP_H_

#include "HConfig.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// The free parser also reads fixed format MPS files but the fixed
// parser does not read free mps files.
enum class HighsMpsParserType { free, fixed };

/** SCIP/HiGHS Objective sense */
enum objSense
{
  OBJSENSE_MINIMIZE = 1,
  OBJSENSE_MAXIMIZE = -1
};


// For now, but later change so HiGHS properties are string based so that new
// options (for debug and testing too) can be added easily. The options below
// are just what has been used to parse options from argv.
// todo: when creating the new options don't forget underscores for class
// variables but no underscores for struct
struct HighsOptions {
  std::string filenames = "";

  bool pami = 0;
  bool sip = 0;
  bool scip = 0;

  // HiGHS run time limit (s): default = 100000? - DBLOPT_TIME_LIMIT
  double timeLimit = 0;

  HighsMpsParserType parser_type = HighsMpsParserType::free;

  std::string presolveMode = "off";
  std::string crashMode = "off";
  std::string edWtMode = "dse2dvx";
  std::string priceMode = "rowswcolsw";
  std::string partitionFile = "";

  // Options not passed through the command line

  // Options for HighsPrintMessage and HighsLogMessage
  // TODO: Use these to set values for use in HighsPrintMessage and HighsLogMessage  
  FILE* output = stdout;
  // HighsPrintMessage level: default = 0 - INTOPT_PRINT_FLAG
  int messageLevel = 0;
  FILE* logfile = stdout;

  // Declare HighsOptions for an LP model, any solver and simplex solver, setting the default value
  //
  // For an LP model
  //
  // Try to solve the dual of the LP: INTOPT_TRANSPOSE_FLAG
  bool transposeLp = false;
  // Perform LP scaling: INTOPT_SCALE_FLAG
  bool scaleLp = true;
  // Permute the columns of the LP randomly to aid load distribution in block parallelism: INTOPT_PERMUTE_FLAG
  bool permuteLp = false;
  // Perform LP bound tightening: INTOPT_TIGHT_FLAG
  bool tightenLp = false;
  //
  // For any solver
  //
  // primal feasibility (dual optimality) tolerance: DBLOPT_PRIMAL_TOL
  double primalFeasibilityTolerance = 1e-7;
  // dual feasibility (primal optimality) tolerance: DBLOPT_DUAL_TOL
  double dualFeasibilityTolerance = 1e-7;
  //
  // For the simplex solver
  //
  // Perturb the original costs: INTOPT_PERTURB_FLAG
  bool perturbCostsSimplex = true;
  // Maximum number of simplex iterations: INTOPT_LPITLIM
  int iterationLimitSimplex = 999999;
  // Upper bound on dual objective value: DBLOPT_OBJ_UB
  double dualObjectiveValueUpperBound = 1e+200;

  bool clean_up = false;
};

class HighsLp {
 public:
  // Model data
  int numCol_ = 0;
  int numRow_ = 0;
  int nnz_ = 0;

  std::vector<int> Astart_;
  std::vector<int> Aindex_;
  std::vector<double> Avalue_;
  std::vector<double> colCost_;
  std::vector<double> colLower_;
  std::vector<double> colUpper_;
  std::vector<double> rowLower_;
  std::vector<double> rowUpper_;

  // sense 1 = minimize, -1 = maximize
  int sense_ = 1;
  double offset_ = 0;
  std::string model_name_ = "";

};

// HiGHS status
enum class HighsStatus {
  OK,
  Init,
  LpError,
  OptionsError,
  PresolveError,
  SolutionError,
  PostsolveError,
  NotImplemented,
  Unbounded,
  Infeasible,
  Feasible,
  Optimal,
  Timeout
};

enum class HighsInputStatus {
  OK,
  FileNotFound,
  ErrorMatrixDimensions,
  ErrorMatrixIndices,
  ErrorMatrixStart,
  ErrorMatrixValue,
  ErrorColBounds,
  ErrorRowBounds,
  ErrorObjective
};

// Cost, column and row scaling factors
struct HighsScale {
  double cost_;
  std::vector<double> col_;
  std::vector<double> row_;
};

struct HighsBasis {
  std::vector<int> basicIndex_;
  std::vector<int> nonbasicFlag_;
  std::vector<int> nonbasicMove_;
};

struct HighsSimplexInfo {
  // Simplex information regarding primal and dual solution, objective
  // and iteration counts for this Highs Model Object. This is
  // information which should be retained from one run to the next in
  // order to provide hot starts.
  //
  // Part of working model which are assigned and populated as much as
  // possible when a model is being defined

  // workCost: Originally just costs from the model but, in solve(), may
  // be perturbed or set to alternative values in Phase I??
  //
  // workDual: Values of the dual variables corresponding to
  // workCost. Latter not known until solve() is called since B^{-1}
  // is required to compute them. Knowledge of them is indicated by
  // mlFg_haveNonbasicDuals.
  //
  // workShift: WTF
  //
  std::vector<double> workCost_;
  std::vector<double> workDual_;
  std::vector<double> workShift_;

  // workLower/workUpper: Originally just lower (upper) bounds from
  // the model but, in solve(), may be perturbed or set to
  // alternative values in Phase I??
  //
  // workRange: Distance between lower and upper bounds
  //
  // workValue: Values of the nonbasic variables corresponding to
  // workLower/workUpper and the basis. Always known.
  //
  std::vector<double> workLower_;
  std::vector<double> workUpper_;
  std::vector<double> workRange_;
  std::vector<double> workValue_;

  // baseLower/baseUpper/baseValue: Lower and upper bounds on the
  // basic variables and their values. Latter not known until solve()
  // is called since B^{-1} is required to compute them. Knowledge of
  // them is indicated by mlFg_haveBasicPrimals.
  //
  std::vector<double> baseLower_;
  std::vector<double> baseUpper_;
  std::vector<double> baseValue_;
  //
  // Vectors of random reals for column cost perturbation, and a
  // random permutation of column indices for shuffling the columns
  // and CHUZR
  std::vector<double> numTotRandomValue_;
  std::vector<int> numColPermutation_;

  // Values of iClock for simplex timing clocks
  std::vector<int> clock_;
  //
  // Value of dual objective
  double dualObjectiveValue;
  // Value of dual objective that is updated in dual simplex solver -
  // need to put this in lower level header, but can't go into Dual.h
  double updatedDualObjectiveValue;

  // Number of simplex iterations: total and constituent counts
  int numberAltIteration;
  int numberAltPhase1DualIteration;
  int numberAltPhase2DualIteration;
  int numberAltPrimalIteration;

  // Options from HighsOptions for the simplex solver
  int simplex_strategy;
  int crash_strategy;
  int dual_edge_weight_strategy;
  int price_strategy;

  double primalFeasibilityTolerance;
  double dualFeasibilityTolerance;
  bool perturbCosts;
  int iterationLimit;
  double dualObjectiveValueUpperBound;
  
  // Internal options - can't be changed externally
  double pamiCutoff;

  // Options for reporting timing
  bool reportSimplexInnerClock;
  bool reportSimplexOuterClock;
  bool reportSimplexPhasesClock;
#ifdef HiGHSDEV
  // Option for analysing simplex iterations, INVERT time and rebuild time
  bool analyseLp;
  bool analyseSimplexIterations;
  bool analyseLpSolution;
  bool analyseInvertTime;
  bool analyseRebuildTime;
#endif
};

struct HighsSolution {
  std::vector<double> colValue_;
  std::vector<double> colDual_;
  std::vector<double> rowValue_;
  std::vector<double> rowDual_;
};

struct HighsRanging {
  std::vector<double> colCostRangeUpValue_;
  std::vector<double> colCostRangeUpObjective_;
  std::vector<int>    colCostRangeUpInCol_;
  std::vector<int>    colCostRangeUpOutCol_;
  std::vector<double> colCostRangeDnValue_;
  std::vector<double> colCostRangeDnObjective_;
  std::vector<int>    colCostRangeDnInCol_;
  std::vector<int>    colCostRangeDnOutCol_;
  std::vector<double> rowBoundRangeUpValue_;
  std::vector<double> rowBoundRangeUpObjective_;
  std::vector<int>    rowBoundRangeUpInCol_;
  std::vector<int>    rowBoundRangeUpOutCol_;
  std::vector<double> rowBoundRangeDnValue_;
  std::vector<double> rowBoundRangeDnObjective_;
  std::vector<int>    rowBoundRangeDnInCol_;
  std::vector<int>    rowBoundRangeDnOutCol_;
};

// Make sure the dimensions of solution are the same as numRow_ and numCol_.
bool isSolutionConsistent(const HighsLp& lp, const HighsSolution& solution);

// Return a string representation of HighsStatus.
std::string HighsStatusToString(HighsStatus status);

// Return a string representation of ParseStatus.
std::string HighsInputStatusToString(HighsInputStatus status);

// If debug this method terminates the program when the status is not OK. If
// standard build it only prints a message.
void checkStatus(HighsStatus status);

HighsInputStatus checkLp(const HighsLp& lp);

#endif
