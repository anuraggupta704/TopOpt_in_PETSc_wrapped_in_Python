#!/usr/bin/python3

# Author: Thijs Smit, May 2020
# Copyright (C) 2020 ETH Zurich

# Disclaimer:
# The authors reserves all rights but does not guaranty that the code is
# free from errors. Furthermore, we shall not be liable in any event
# caused by the use of the program.

import topoptlib
import numpy as np

# step 1:
# Create data class to store input data
data = topoptlib.Data()

# step 2:
# define input data
# mesh: (domain: x, y, z, center)(mesh: number of nodes)
#data.structuredGrid((0.0, 192.0, 0.0, 64.0, 0.0, 104.0, 0.0, 0.0, 0.0), (193, 65, 105))
data.structuredGrid((0.0, 192.0, 0.0, 64.0, 0.0, 104.0, 0.0, 0.0, 0.0), (385, 129, 209))

# readin STL file in binary format
# TO DO: allow for ASCII format
# stl read: ((box around stl: (min corner)(max corner))full path to file)
data.stlread_domain((-23.0, -1.0, -103.0), (169.0, 63.0, 1.0), '/cluster/home/thsmit/TopOpt_in_PETSc_wrapped_in_Python/bracket/jetEngineDesignDomainFine.stl')

# stl read: load the solid domain into the same coordinate system as the design domain
data.stlread_solid('/cluster/home/thsmit/TopOpt_in_PETSc_wrapped_in_Python/bracket/jetEngineSolidDomainFine.stl')

# stl read: load the solid domain into the same coordinate system as the design domain
data.stlread_rigid('/cluster/home/thsmit/TopOpt_in_PETSc_wrapped_in_Python/bracket/jetEngineRigidDomainFine.stl')

# Optional printing:
#print(data.nNodes)
print(data.nElements)
#print(data.nDOF)

'''
# material: (Emin, Emax, nu, penal)
Emin, Emax, nu, Dens, penal = 1.0e-9, 1.0, 0.3, 1.0, 1.0
data.material(Emin, Emax, nu, Dens, penal)

# filter: (type, radius)
# filter types: sensitivity = 0, density = 1, 
data.filter(1, 0.001)

# optimizer: (maxIter)
data.mma(10000)

# loadcases: (# of loadcases)
data.loadcases(1)

# bc: (loadcase, type, [checker: lcoorp[i+?], xc[?]], [setter: dof index], [setter: values])
data.bc(0, 1, [0, 0], [0, 1, 2], [0.0, 0.0, 0.0], 0)
data.bc(0, 2, [0, 1, 2, 4], [2], [-0.001], 0)
data.bc(0, 2, [0, 1, 1, 2, 2, 4], [2], [-0.0005], 0)
data.bc(0, 2, [0, 1, 1, 3, 2, 4], [2], [-0.0005], 0)

#data.bc(1, 1, [[0, 0]], [0, 1, 2], [0.0, 0.0, 0.0], None)
#data.bc(1, 2, [[0, 1], [1, 3]], [1], [0.001], None)
#data.bc(1, 2, [[0, 1], [1, 3], [2, 4]], [1], [0.0005], None)
#data.bc(1, 2, [[0, 1], [1, 3], [2, 5]], [1], [0.0005], None)

materialvolumefraction = 0.40
nEl = data.nElements

# Calculate the objective function
# objective input: (design variable value, SED)
def objective(sumXp, xp, uKu):
    return (Emin + np.power(xp, penal) * (Emax - Emin)) * uKu

def sensitivity(sumXp, xp, uKu):
    return -1.0 * penal * np.power(xp, (penal - 1)) * (Emax - Emin) * uKu

def constraint(sumXp, xp, uKu):
    return (sumXp / nEl - materialvolumefraction) / nEl

def constraintSensitivity(sumXp, xp, uKu):
    return 1.0 / nEl

# Callback implementation
data.obj(objective)
data.objsens(sensitivity)

# Define constraint
data.cons(constraint)
data.conssens(constraintSensitivity)

# Volume constraint is standard, input (volume fraction)
data.volumeConstraint(materialvolumefraction)

# step 3:
# solve topopt problem with input data and wait for "complete" signal
complete = data.solve()

# step 4:
# post processing, generate .vtu file to be viewed in paraview
#if complete:
#    data.vtu()
'''
