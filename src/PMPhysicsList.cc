#include "PMPhysicsList.hh"

PMPhysicsList::PMPhysicsList()
{
    // EM Physics
    RegisterPhysics(new G4EmStandardPhysics());

    // Radioactive decay physics
    RegisterPhysics(new G4RadioactiveDecayPhysics());

    // Decay physics
    RegisterPhysics(new G4DecayPhysics());

    //Optics
    RegisterPhysics(new G4OpticalPhysics());
}

PMPhysicsList::~PMPhysicsList()
{
}