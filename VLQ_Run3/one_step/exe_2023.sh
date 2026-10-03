#!/bin/bash -x

env

JOBNUM=${1##*=}
NEVENT=${2##*=}
NTHREAD=${3##*=}
PROCNAME=${4##*=}
BEGINSEED=${5##*=}

WORKDIR=`pwd`

export SCRAM_ARCH=el9_amd64_gcc11
export RELEASE=CMSSW_13_0_13
source /cvmfs/cms.cern.ch/cmsset_default.sh

if [ -r $RELEASE/src ] ; then
  echo release $RELEASE already exists
else
  scram p CMSSW $RELEASE
fi
cd $RELEASE/src
eval `scram runtime -sh`

mkdir -p Configuration/GenProduction/python/
cp $WORKDIR/inputs/${PROCNAME}.py Configuration/GenProduction/python/${PROCNAME}.py
sed "s/__NEVENT__/$NEVENT/g" -i Configuration/GenProduction/python/${PROCNAME}.py
scram b -j $NTHREAD

cd $WORKDIR

SEED=$(((${BEGINSEED} + ${JOBNUM}) * 100))

# Step 1: LHE + GEN + SIM
cmsDriver.py Configuration/GenProduction/python/${PROCNAME}.py \
  --python_filename wmLHEGEN_cfg.py \
  --eventcontent RAWSIM \
  --customise Configuration/DataProcessing/Utils.addMonitoring \
  --datatier GEN-SIM \
  --fileout file:sim.root \
  --conditions 130X_mcRun3_2023_realistic_v15 \
  --beamspot Realistic25ns13p6TeVEarly2023Collision \
  --customise_commands process.RandomNumberGeneratorService.externalLHEProducer.initialSeed="int(${SEED})"\\nprocess.source.numberEventsInLuminosityBlock="cms.untracked.uint32(100)" \
  --step LHE,GEN,SIM \
  --geometry DB:Extended \
  --era Run3_2023 \
  --mc --nThreads $NTHREAD -n $NEVENT || exit $? ;

# Step 2: DIGI + DATAMIX + L1 + DIGI2RAW + HLT (premix)
cmsDriver.py \
  --python_filename DIGIPremix_cfg.py \
  --eventcontent PREMIXRAW \
  --customise Configuration/DataProcessing/Utils.addMonitoring \
  --datatier GEN-SIM-RAW \
  --fileout file:digi.root \
  --pileup_input "dbs:/Neutrino_E-10_gun/Run3Summer21PrePremix-Summer23_130X_mcRun3_2023_realistic_v13-v1/PREMIX" \
  --conditions 130X_mcRun3_2023_realistic_v14 \
  --step DIGI,DATAMIX,L1,DIGI2RAW,HLT:2023v12 \
  --procModifiers premix_stage2 \
  --geometry DB:Extended \
  --filein file:sim.root \
  --datamix PreMix \
  --era Run3_2023 \
  --mc --nThreads $NTHREAD -n $NEVENT || exit $? ;

# Step 3: RECO
cmsDriver.py \
  --python_filename RECO_cfg.py \
  --eventcontent AODSIM \
  --customise Configuration/DataProcessing/Utils.addMonitoring \
  --datatier AODSIM \
  --fileout file:reco.root \
  --conditions 130X_mcRun3_2023_realistic_v15 \
  --step RAW2DIGI,L1Reco,RECO,RECOSIM \
  --geometry DB:Extended \
  --filein file:digi.root \
  --era Run3_2023 \
  --mc --nThreads $NTHREAD -n $NEVENT || exit $? ;

# Step 4: MiniAOD
cmsDriver.py \
  --python_filename MiniAOD_cfg.py \
  --eventcontent MINIAODSIM \
  --customise Configuration/DataProcessing/Utils.addMonitoring \
  --datatier MINIAODSIM \
  --fileout file:mini.root \
  --conditions 130X_mcRun3_2023_realistic_v15 \
  --step PAT \
  --geometry DB:Extended \
  --filein file:reco.root \
  --era Run3_2023 \
  --mc --no_exec --nThreads $NTHREAD -n $NEVENT || exit $? ;
cmsRun -j FrameworkJobReport.xml MiniAOD_cfg.py

# Step 5: NanoAOD
cmsDriver.py \
  --python_filename NanoAOD_cfg.py \
  --eventcontent NANOAODSIM \
  --customise Configuration/DataProcessing/Utils.addMonitoring \
  --datatier NANOAODSIM \
  --fileout file:nano.root \
  --conditions 130X_mcRun3_2023_realistic_v15 \
  --step NANO \
  --filein file:mini.root \
  --era Run3_2023 \
  --mc --no_exec --nThreads $NTHREAD -n $NEVENT || exit $? ;
cmsRun -j FrameworkJobReport.xml NanoAOD_cfg.py
