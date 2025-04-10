#include "PMDetectorConstruction.hh"

PMDetectorConstruction::PMDetectorConstruction()
{
}

PMDetectorConstruction::~PMDetectorConstruction()
{
}

G4VPhysicalVolume *PMDetectorConstruction::Construct()
{
    G4bool checkOverlaps = true;

    G4NistManager *nist  = G4NistManager::Instance();

    //gas at very low density ~ vacuum
    G4double atomicNumber = 1.;
    G4double massOfMole = 1.008*g/mole;
    G4double density = 1.e-25*g/cm3;
    G4double temperature = 2.73*kelvin;
    G4double pressure = 3.e-18*pascal;

    G4Material *worldMat = new G4Material("vacuum", atomicNumber, massOfMole, density,
                                            kStateGas, temperature, pressure);
    
    G4double rindexWorld[2] = {1.0, 1.0};

    //BC-408 scintillator
    G4Material* BC408 = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    G4double energyBC408[2] = {2.95*eV, 3.10*eV};
    G4double rindexBC408[2] = {1.58, 1.58};
    G4double absorptionBC408[2] = {210*cm, 210*cm};
    G4double emissionBC408[2] = {1.0, 1.0};

    //aluminum + mylar radiator

    radiatorSurface = new G4OpticalSurface("radiatorSurface");

    radiatorSurface->SetType(dielectric_metal);
    radiatorSurface->SetFinish(groundfrontpainted);
    radiatorSurface->SetModel(unified);
    
    G4double energyRad[2] = {1.5*eV, 6.2*eV};
    G4double reflectivityRad[2] = {0.97, 0.97};
    G4double efficiencyRad[2]   = {1.0, 1.0};

    G4double specularLobe[] = {0.3, 0.3};
    G4double specularSpike[] = {0.2, 0.2};
    G4double backscatter[] = {0.05, 0.05};

    // G4Material *aluminumMat = nist->FindOrBuildMaterial("G4_Al");
    // G4Material *mylarMat = nist->FindOrBuildMaterial("G4_MYLAR");

    //glass PMT
    G4Material *glassMat = nist->FindOrBuildMaterial("G4_GLASS_PLATE");

    G4double rindexGlass[2] = {1.5, 1.5};
    G4double absorptionGlass[2] = {0.1*cm, 0.1*cm};

    //material properties tables
    
    G4MaterialPropertiesTable *mptWorld = new G4MaterialPropertiesTable();
    mptWorld->AddProperty("RINDEX", energyRad, rindexWorld, 2);
    worldMat->SetMaterialPropertiesTable(mptWorld);

    G4MaterialPropertiesTable *mptBC408 = new G4MaterialPropertiesTable();
    mptBC408->AddProperty("RINDEX", energyBC408, rindexBC408, 2);
    mptBC408->AddProperty("SCINTILLATIONCOMPONENT1", energyBC408, emissionBC408, 2);
    mptBC408->AddProperty("ABSLENGTH", energyBC408, absorptionBC408, 2);
    mptBC408->AddConstProperty("SCINTILLATIONYIELD", 10000.0 / MeV);
    mptBC408->AddConstProperty("RESOLUTIONSCALE", 1.0);
    mptBC408->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.1 * ns);
    mptBC408->AddConstProperty("SCINTILLATIONYIELD1", 1.0, true);
    BC408->SetMaterialPropertiesTable(mptBC408);

    G4MaterialPropertiesTable* radiatorMPT = new G4MaterialPropertiesTable();
    radiatorMPT->AddProperty("REFLECTIVITY", energyRad, reflectivityRad, 2);
    radiatorMPT->AddProperty("EFFICIENCY", energyRad, efficiencyRad, 2);
    radiatorMPT->AddProperty("SPECULARLOBECONSTANT", energyRad, specularLobe, 2);
    radiatorMPT->AddProperty("SPECULARSPIKECONSTANT", energyRad, specularSpike, 2);
    radiatorMPT->AddProperty("BACKSCATTERCONSTANT", energyRad, backscatter, 2);
    radiatorSurface->SetMaterialPropertiesTable(radiatorMPT);

    //world construction

    G4double xWorld = 1. * m;
    G4double yWorld = 1. * m;
    G4double zWorld = 1. * m;

    G4Box *solidWorld = new G4Box("solidWorld", 0.5 * xWorld, 0.5 * yWorld, 0.5 * zWorld);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, worldMat, "logicalWorld");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld, "physWorld", 0, false, 0, checkOverlaps);

    //scintillator construction

    G4Box* solidScint = new G4Box("scintillator", 30*cm, 30*cm, 1*cm);
    G4LogicalVolume* logicScint[3];
    G4PVPlacement* physScint[3];

    G4VisAttributes *scintVisAtt = new G4VisAttributes(G4Color(1.0, 0.0, 1.0, 0.5));
    scintVisAtt->SetForceSolid(true);

    for (int i = 0; i < 3; ++i) {
        logicScint[i] = new G4LogicalVolume(solidScint, BC408, "logicScint"+std::to_string(i));
        G4double zPos = i * 20*cm - 30*cm;

        physScint[i] = new G4PVPlacement(0, G4ThreeVector(0, 0, zPos), logicScint[i], "physScint"+std::to_string(i), logicWorld, false, i);
        
        G4LogicalSkinSurface *skin = new G4LogicalSkinSurface("skin", logicScint[i], radiatorSurface);
        logicScint[i]->SetVisAttributes(scintVisAtt);
    }

    //PMT construction
    G4Box* solidPMT = new G4Box("PMT", 30*cm, 30*cm, 0.5*cm);
    G4LogicalVolume* logicPMT[3];
    G4PVPlacement* physPMT[3];

    G4VisAttributes *PMTVisAtt = new G4VisAttributes(G4Color(1.0, 0.0, 0.0, 0.5));
    PMTVisAtt->SetForceSolid(true);

    for (int i = 0; i < 3; ++i) {
        logicPMT[i] = new G4LogicalVolume(solidPMT, glassMat, "logicPMT"+std::to_string(i));
        G4double zPos = i * 20*cm - 31*cm;

        physPMT[i] = new G4PVPlacement(0, G4ThreeVector(0, 0, zPos), logicPMT[i], "physPMT"+std::to_string(i), logicWorld, false, i);

        PMSensitiveDetector* pmSD = new PMSensitiveDetector("PMTSD"+std::to_string(i));
        G4SDManager::GetSDMpointer()->AddNewDetector(pmSD);

        logicPMT[i]->SetSensitiveDetector(pmSD);
        logicPMT[i]->SetVisAttributes(PMTVisAtt);
        
    }

    return physWorld;
}

void PMDetectorConstruction::ConstructSDandField()
{
    PMSensitiveDetector *sensDet = new PMSensitiveDetector("SensitveDetector");
    logicDetector->SetSensitiveDetector(sensDet);
    G4SDManager::GetSDMpointer()->AddNewDetector(sensDet);
}