/*
Original Author:  Davide Di Croce
         Created:  February 2021
*/

// System include files
#include <memory>
#include <vector>
#include <cmath>  // For std::atan2, std::sin, std::cos, and std::sqrt
#include <iostream> // For printing

// User include files
#include "FWCore/Framework/interface/global/EDFilter.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"

// Class declaration
class GenTToHadronicFilter : public edm::global::EDFilter<> {
public:
  explicit GenTToHadronicFilter(const edm::ParameterSet&);

private:
  bool filter(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  // Member data
  const edm::EDGetTokenT<reco::GenParticleCollection> token_;
  const double quarkPtCut_, quarkEtaCut_, nTops_, wbdRCut_;

  // Calculate delta R
  //double calculate_delta_R(double eta1, double phi1, double eta2, double phi2) const {
    //double delta_eta = eta1 - eta2;
    //double delta_phi = std::atan2(std::sin(phi1 - phi2), std::cos(phi1 - phi2));
    //return std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);
 //} 
};

// Constructor
GenTToHadronicFilter::GenTToHadronicFilter(const edm::ParameterSet& params)
    : token_(consumes<reco::GenParticleCollection>(params.getParameter<edm::InputTag>("src"))),
      quarkPtCut_(params.getParameter<double>("quarkPtCut")),
      quarkEtaCut_(params.getParameter<double>("quarkEtaCut")),
      nTops_(params.getParameter<double>("nTops")),
      wbdRCut_(params.getParameter<double>("wbdRCut")) {}

std::vector<int> quarkIDs = {1, 2, 3, 4, 5, 6, -1, -2, -3, -4 ,-5, -6};
bool GenTToHadronicFilter::filter(edm::StreamID, edm::Event& evt, const edm::EventSetup& params) const {
    using namespace std;
    using namespace edm;
    using namespace reco;

    // Read GenParticles Collection from Event
    Handle<GenParticleCollection> genParticles;
    evt.getByToken(token_, genParticles);

    // Loop over all elements in Event
    unsigned TToHadronicCandidate = 0;
    unsigned totalParticles = 0;
    unsigned countNonTop = 0;
    unsigned countWrongNumDaughters = 0;


    for (const auto& iGen : *genParticles) {
        ++totalParticles; // Count total particles processed

        // Filter for top quarks only
        if (abs(iGen.pdgId()) != 6) {
            ++countNonTop; //Skips particles that are Not a top
            continue;
        }

        // Ensure the top quark has exactly two daughters
        if (iGen.numberOfDaughters() != 2) {
            ++countWrongNumDaughters; // Skips particles having more than 2 daughter or less
            continue;
        }

        // Increment counter if both filters are passed
        ++TToHadronicCandidate;
    }


    // Print the summary of cuts
    std::cout << "Total Particles Processed: " << totalParticles << std::endl;
    std::cout << "Cut at Top Selection mismatch: " << countNonTop << std::endl;
   // std::cout << "Cut at B-Quark Selection: " << cutAtBQuark << std::endl;
   //std::cout << "Cut at W and B Delta R: " << cutAtWandBDeltaR << std::endl;
    std::cout << "Cut at Not WB daughter selection: " << countWrongNumDaughters << std::endl;
    std::cout << "Passed Candidates: " << TToHadronicCandidate << std::endl;

    return (TToHadronicCandidate >= nTops_);
}

// Define module as a plug-in
DEFINE_FWK_MODULE(GenTToHadronicFilter);

