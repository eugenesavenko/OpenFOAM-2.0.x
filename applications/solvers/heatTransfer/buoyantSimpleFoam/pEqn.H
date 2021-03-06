{
    rho = thermo.rho();
    rho.relax();

    volScalarField rAU(1.0/UEqn().A());
    surfaceScalarField rhorAUf("(rho*(1|A(U)))", fvc::interpolate(rho*rAU));

    U = rAU*UEqn().H();
    UEqn.clear();

    phi = fvc::interpolate(rho)*(fvc::interpolate(U) & mesh.Sf());
    bool closedVolume = adjustPhi(phi, U, p_rgh);

    surfaceScalarField buoyancyPhi(rhorAUf*ghf*fvc::snGrad(rho)*mesh.magSf());
    phi -= buoyancyPhi;

    for (int nonOrth=0; nonOrth<=simple.nNonOrthCorr(); nonOrth++)
    {
        fvScalarMatrix p_rghEqn
        (
            fvm::laplacian(rhorAUf, p_rgh) == fvc::div(phi)
        );

        p_rghEqn.setReference(pRefCell, getRefCellValue(p_rgh, pRefCell));
        p_rghEqn.solve();

        if (nonOrth == simple.nNonOrthCorr())
        {
            // Calculate the conservative fluxes
            phi -= p_rghEqn.flux();

            // Explicitly relax pressure for momentum corrector
            p_rgh.relax();

            // Correct the momentum source with the pressure gradient flux
            // calculated from the relaxed pressure
            U -= rAU*fvc::reconstruct((buoyancyPhi + p_rghEqn.flux())/rhorAUf);
            U.correctBoundaryConditions();
        }
    }

    #include "continuityErrs.H"

    p = p_rgh + rho*gh;

    // For closed-volume cases adjust the pressure level
    // to obey overall mass continuity
    if (closedVolume)
    {
        p += (initialMass - fvc::domainIntegrate(psi*p))
            /fvc::domainIntegrate(psi);
        p_rgh = p - rho*gh;
    }

    rho = thermo.rho();
    rho.relax();
    Info<< "rho max/min : " << max(rho).value() << " " << min(rho).value()
        << endl;
}
