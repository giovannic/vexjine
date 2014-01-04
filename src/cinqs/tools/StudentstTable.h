#ifndef TOOLS_STUDENTSTTABLE_H
#define TOOLS_STUDENTSTTABLE_H


class StudentstTable {

public:
	static bool checkConfidenceLevel( double alpha ) ;
	static bool checkDegreesOfFreedom( int dof ) ;
	static double table ( int dof, double alpha ) ;

	static double alphas[4];
	static int minDof;
	static int maxDof;

	static double t01[21];

	static double t025[21];
	static double t05[21];

	static double t10[21];
};

#endif
