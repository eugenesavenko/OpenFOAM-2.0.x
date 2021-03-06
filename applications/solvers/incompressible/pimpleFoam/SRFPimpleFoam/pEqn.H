volScalarField rAUrel(1.0/UrelEqn().A());
Urel = rAUrel*UrelEqn().H();

if (pimple.nCorr() <= 1)
{
    UrelEqn.clear();
}

phi = (fvc::interpolate(Urel) & mesh.Sf())
    + fvc::ddtPhiCorr(rAUrel, Urel, phi);

adjustPhi(phi, Urel, p);

// Non-orthogonal pressure corrector loop
for (int nonOrth=0; nonOrth<=pimple.nNonOrthCorr(); nonOrth++)
{
    // Pressure corrector
    fvScalarMatrix pEqn
    (
        fvm::laplacian(rAUrel, p) == fvc::div(phi)
    );

    pEqn.setReference(pRefCell, pRefValue);

    pEqn.solve
    (
        mesh.solver(p.select(pimple.finalInnerIter(corr, nonOrth)))
    );

    if (nonOrth == pimple.nNonOrthCorr())
    {
        phi -= pEqn.flux();
    }
}

#include "continuityErrs.H"

p.relax();

// Momentum corrector
Urel -= rAUrel*fvc::grad(p);
Urel.correctBoundaryConditions();
