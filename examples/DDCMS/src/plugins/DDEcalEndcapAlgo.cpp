#include "DD4hep/DetFactoryHelper.h"
#include "DDCMS/DDCMSPlugins.h"
#include <Math/AxisAngle.h>
#include <Math/Rotation3D.h>
#include <Math/Vector3D.h>

#include <CLHEP/Geometry/Transform3D.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Geometry/Plane3D.h>
#include <CLHEP/Geometry/Vector3D.h>
#include <CLHEP/Geometry/Transform3D.h>
#include <CLHEP/Vector/EulerAngles.h>

#include <array>
#include <string>
#include <vector>

using namespace std;
using namespace dd4hep;

using DD3Vector = ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double>>;
using DDTranslation = ROOT::Math::DisplacementVector3D<ROOT::Math::Cartesian3D<double> >;
using DDRotation = ROOT::Math::Rotation3D;
using DDRotationMatrix = ROOT::Math::Rotation3D;
using DDAxisAngle = ROOT::Math::AxisAngle;

namespace {

  constexpr long double piRadians(M_PI);
  constexpr long double degPerRad = 180. / piRadians;  // Degrees per radian
  constexpr double operator""_mm(long double length) { return length * 0.1; }
  constexpr long double operator""_deg(long double value) { return value / degPerRad; }

  // Define Endcap Supercrystal class
  class DDEcalEndcapTrap {
  public:
    DDEcalEndcapTrap(const int hand, const double front, const double rear, const double length);
    DDEcalEndcapTrap() = delete;

    void rotate(const DDRotationMatrix& rot);
    //void rotate(const DDTranslation& frontCentre, const DDTranslation& rearCentre);
    void translate(const DDTranslation& trans);

    void rotateX(const double angle);
    void rotateY(const double angle);
    void translate();
    void moveto(const DDTranslation& frontCentre, const DDTranslation& rearCentre);
    double elevationAngle(const DDTranslation& trans);
    double polarAngle(const DDTranslation& trans);
    double elevationAngle();
    double polarAngle();
    DDTranslation cornerPos(const int icorner);
    void cornerPos(const int icorner, const DDTranslation& cc);
    DDTranslation centrePos();
    DDTranslation fcentrePos();
    DDTranslation rcentrePos();
    void calculateCorners();
    void calculateCentres();
    DDRotationMatrix rotation() { return m_rotation; }
    //void print();

  private:
    DDRotationMatrix m_rotation;
    DDTranslation m_translation;

    double m_centre[4];
    double m_fcentre[4];
    double m_rcentre[4];
    double m_corners[25];
    double m_front;
    double m_rear;
    double m_length;

    int m_hand;
    int m_update;
  };
  // Implementation of DDEcalEndcapTrap class

  DDEcalEndcapTrap::DDEcalEndcapTrap(const int hand, const double front, const double rear, const double length) {
    //
    //  Initialise corners of supercrystal.

    // Start out with bottom surface on (x,z) plane, front face in (x,y) plane.

    double xsign;

    if (hand == 2) {
      xsign = -1.;
    } else {
      xsign = 1.;
    }

    m_hand = hand;
    m_front = front;
    m_rear = rear;
    m_length = length;

    int icorner;
    icorner = 1;
    m_corners[3 * icorner - 3] = xsign * front;
    m_corners[3 * icorner - 2] = front;
    m_corners[3 * icorner - 1] = 0.;
    icorner = 2;
    m_corners[3 * icorner - 3] = xsign * front;
    m_corners[3 * icorner - 2] = 0.;
    m_corners[3 * icorner - 1] = 0.;
    icorner = 3;
    m_corners[3 * icorner - 3] = 0.;
    m_corners[3 * icorner - 2] = 0.;
    m_corners[3 * icorner - 1] = 0.;
    icorner = 4;
    m_corners[3 * icorner - 3] = 0.;
    m_corners[3 * icorner - 2] = front;
    m_corners[3 * icorner - 1] = 0.;

    icorner = 5;
    m_corners[3 * icorner - 3] = xsign * rear;
    m_corners[3 * icorner - 2] = rear;
    m_corners[3 * icorner - 1] = length;
    icorner = 6;
    m_corners[3 * icorner - 3] = xsign * rear;
    m_corners[3 * icorner - 2] = 0.;
    m_corners[3 * icorner - 1] = length;
    icorner = 7;
    m_corners[3 * icorner - 3] = 0.;
    m_corners[3 * icorner - 2] = 0.;
    m_corners[3 * icorner - 1] = length;
    icorner = 8;
    m_corners[3 * icorner - 3] = 0.;
    m_corners[3 * icorner - 2] = rear;
    m_corners[3 * icorner - 1] = length;

    calculateCentres();

    // Move centre of SC to (0,0,0)

    translate();

    // Rotate into standard position (face centres on z axis)

    //  this->rotate();

    calculateCentres();
  }
#if 0
  void DDEcalEndcapTrap::rotate(const DDTranslation& /* frontCentre */, const DDTranslation& /* rearCentre */) {
    //
    //  Rotate supercrystal to bring front and rear face centres to specified points
    //
  }
#endif
  void DDEcalEndcapTrap::rotate(const DDRotationMatrix& rot) {
    //
    //  Rotate supercrystal by specified rotation about (0,0,0)
    //

    int icorner;
    DDTranslation cc;
    for (icorner = 1; icorner <= 8; icorner++) {
      cc = cornerPos(icorner);
      cc = rot * cc;
      cornerPos(icorner, cc);
    }
    m_rotation = rot * m_rotation;
    calculateCentres();
  }

  void DDEcalEndcapTrap::translate() {
    translate(-1. * centrePos());
  }

  void DDEcalEndcapTrap::translate(const DDTranslation& trans) {
    //
    //  Translate supercrystal by specified amount
    //

    DDTranslation tcorner;
    for (int icorner = 1; icorner <= 8; icorner++) {
      tcorner = cornerPos(icorner) + trans;
      cornerPos(icorner, tcorner);
    }
    calculateCentres();
    m_translation = trans + m_translation;
  }

  void DDEcalEndcapTrap::moveto(const DDTranslation& frontCentre, const DDTranslation& rearCentre) {
    //
    //  Rotate (about X then about Y) and translate supercrystal to bring axis joining front and rear face centres parallel to line connecting specified points
    //

    //  Get azimuthal and polar angles of current axis and target axis
    double currentTheta = elevationAngle();
    double currentPhi = polarAngle();
    double targetTheta = elevationAngle(frontCentre - rearCentre);
    double targetPhi = polarAngle(frontCentre - rearCentre);

    //  Rotate to correct angle (X then Y)
    rotateX(targetTheta - currentTheta);
    rotateY(targetPhi - currentPhi);

    //  Translate SC to final position
    DDTranslation targetCentre = 0.5 * (frontCentre + rearCentre);
    translate(targetCentre - centrePos());
  }

  void DDEcalEndcapTrap::rotateX(const double angle) {
    //
    //  Rotate SC through given angle about X axis
    //

    const CLHEP::HepRotation tmp(CLHEP::Hep3Vector(1., 0., 0.), angle);

    rotate(DDRotationMatrix(tmp.xx(), tmp.xy(), tmp.xz(), tmp.yx(), tmp.yy(), tmp.yz(), tmp.zx(), tmp.zy(), tmp.zz()));
  }

  void DDEcalEndcapTrap::rotateY(const double angle) {
    //
    //  Rotate SC through given angle about Y axis
    //
    const CLHEP::HepRotation tmp(CLHEP::Hep3Vector(0., 1., 0.), angle);

    rotate(DDRotationMatrix(tmp.xx(), tmp.xy(), tmp.xz(), tmp.yx(), tmp.yy(), tmp.yz(), tmp.zx(), tmp.zy(), tmp.zz()));
  }

  void DDEcalEndcapTrap::calculateCentres() {
    //
    //  Calculate crystal centre and front & rear face centres
    //

    int ixyz, icorner;

    for (ixyz = 0; ixyz < 3; ixyz++) {
      m_centre[ixyz] = 0;
      m_fcentre[ixyz] = 0;
      m_rcentre[ixyz] = 0;
    }

    for (icorner = 1; icorner <= 4; icorner++) {
      for (ixyz = 0; ixyz < 3; ixyz++) {
	m_centre[ixyz] = m_centre[ixyz] + 0.125 * m_corners[3 * icorner - 3 + ixyz];
	m_fcentre[ixyz] = m_fcentre[ixyz] + 0.25 * m_corners[3 * icorner - 3 + ixyz];
      }
    }
    for (icorner = 5; icorner <= 8; icorner++) {
      for (ixyz = 0; ixyz < 3; ixyz++) {
	m_centre[ixyz] = m_centre[ixyz] + 0.125 * m_corners[3 * icorner - 3 + ixyz];
	m_rcentre[ixyz] = m_rcentre[ixyz] + 0.25 * m_corners[3 * icorner - 3 + ixyz];
      }
    }
  }

  DDTranslation DDEcalEndcapTrap::cornerPos(const int icorner) {
    //
    //  Return specified corner as a DDTranslation
    //
    return DDTranslation(m_corners[3 * icorner - 3], m_corners[3 * icorner - 2], m_corners[3 * icorner - 1]);
  }

  void DDEcalEndcapTrap::cornerPos(const int icorner, const DDTranslation& cornerxyz) {
    //
    //  Save position of specified corner.
    //
    for (int ixyz = 0; ixyz < 3; ixyz++) {
      m_corners[3 * icorner - 3 + ixyz] = (0 == ixyz ? cornerxyz.x() : (1 == ixyz ? cornerxyz.y() : cornerxyz.z()));
      ;
    }
  }

  DDTranslation DDEcalEndcapTrap::centrePos() {
    //
    //  Return SC centre as a DDTranslation
    //
    return DDTranslation(m_centre[0], m_centre[1], m_centre[2]);
  }

  DDTranslation DDEcalEndcapTrap::fcentrePos() {
    //
    //  Return SC front face centre as a DDTranslation
    //
    return DDTranslation(m_fcentre[0], m_fcentre[1], m_fcentre[2]);
  }

  DDTranslation DDEcalEndcapTrap::rcentrePos() {
    //
    //  Return SC rear face centre as a DDTranslation
    //
    return DDTranslation(m_rcentre[0], m_rcentre[1], m_rcentre[2]);
  }

  double DDEcalEndcapTrap::elevationAngle(const DDTranslation& trans) {
    //
    //  Return elevation angle (out of x-z plane) of a given translation (seen as a vector from the origin).
    //
    double sintheta = trans.y() / trans.r();
    return asin(sintheta);
  }

  double DDEcalEndcapTrap::elevationAngle() {
    //
    //  Return elevation angle (out of x-z plane) of SC in current position.
    //
    DDTranslation current = fcentrePos() - rcentrePos();
    return elevationAngle(current);
  }

  double DDEcalEndcapTrap::polarAngle(const DDTranslation& trans) {
    //
    //  Return polar angle (from x to z) of a given translation (seen as a vector from the origin).
    //
    double tanphi = trans.x() / trans.z();
    return atan(tanphi);
  }

  double DDEcalEndcapTrap::polarAngle() {
    //
    //  Return elevation angle (out of x-z plane) of SC in current position.
    //
    DDTranslation current = fcentrePos() - rcentrePos();
    return polarAngle(current);
  }
#if 0
  void DDEcalEndcapTrap::print() {
    //
    //  Print SC coordinates for debugging
    //
    for (int ic = 1; ic <= 8; ic++) {
      /* DDTranslation cc = */  cornerPos(ic);
    }
  }
#endif
  namespace {
    struct Endcap {
      string mat;
      double zOff;

      string quaName;
      string quaMat;

      string crysMat;
      string wallMat;

      double crysLength;
      double crysRear;
      double crysFront;
      double sCELength;
      double sCERear;
      double sCEFront;
      double sCALength;
      double sCARear;
      double sCAFront;
      double sCAWall;
      double sCHLength;
      double sCHSide;

      double nSCTypes;
      vector<double> vecEESCProf;
      double nColumns;
      vector<double> vecEEShape;
      double nSCCutaway;
      vector<double> vecEESCCutaway;
      double nSCquad;
      vector<double> vecEESCCtrs;
      double nCRSC;
      vector<double> vecEECRCtrs;

      array<double, 3> cutParms;
      string cutBoxName;

      string envName;
      string alvName;
      string intName;
      string cryName;

      DDTranslation cryFCtr[5][5];
      DDTranslation cryRCtr[5][5];
      DDTranslation scrFCtr[10][10];
      DDTranslation scrRCtr[10][10];

      double pFHalf;
      double pFFifth;
      double pF45;

      vector<double> vecEESCLims;

      double iLength;
      double iXYOff;
      double cryZOff;
      double zFront;
    };

    const Rotation3D& myrot(dd4hep::cms::Namespace& ns, const string& nam, const Rotation3D& r) {
      ns.addRotation(nam, r);
      return ns.rotation(ns.prepend(nam));
    }

    string_view mynamespace(string_view input) {
      string_view v = input;
      auto trim_pos = v.find(':');
      if (trim_pos != v.npos)
	v.remove_suffix(v.size() - (trim_pos + 1));
      return v;
    }
  }  // namespace

  static long algorithm(dd4hep::Detector& /* description */, dd4hep::cms::ParsingContext& ctxt, xml_h e,
			SensitiveDetector& /* sens */) {
    dd4hep::cms::Namespace ns(ctxt, e, true);
    dd4hep::cms::AlgoArguments args(ctxt, e);

    // TRICK!
    string myns{mynamespace(args.parentName()).data(), mynamespace(args.parentName()).size()};

    Endcap ee;
    ee.mat = args.str("EEMat");
    ee.zOff = args.dble("EEzOff");

    ee.quaName = args.str("EEQuaName");
    ee.quaMat = args.str("EEQuaMat");
    ee.crysMat = args.str("EECrysMat");
    ee.wallMat = args.str("EEWallMat");
    ee.crysLength = args.dble("EECrysLength");
    ee.crysRear = args.dble("EECrysRear");
    ee.crysFront = args.dble("EECrysFront");
    ee.sCELength = args.dble("EESCELength");
    ee.sCERear = args.dble("EESCERear");
    ee.sCEFront = args.dble("EESCEFront");
    ee.sCALength = args.dble("EESCALength");
    ee.sCARear = args.dble("EESCARear");
    ee.sCAFront = args.dble("EESCAFront");
    ee.sCAWall = args.dble("EESCAWall");
    ee.sCHLength = args.dble("EESCHLength");
    ee.sCHSide = args.dble("EESCHSide");
    ee.nSCTypes = args.dble("EEnSCTypes");
    ee.nColumns = args.dble("EEnColumns");
    ee.nSCCutaway = args.dble("EEnSCCutaway");
    ee.nSCquad = args.dble("EEnSCquad");
    ee.nCRSC = args.dble("EEnCRSC");
    ee.vecEESCProf = args.vecDble("EESCProf");
    ee.vecEEShape = args.vecDble("EEShape");
    ee.vecEESCCutaway = args.vecDble("EESCCutaway");
    ee.vecEESCCtrs = args.vecDble("EESCCtrs");
    ee.vecEECRCtrs = args.vecDble("EECRCtrs");

    ee.cutBoxName = args.str("EECutBoxName");

    ee.envName = args.str("EEEnvName");
    ee.alvName = args.str("EEAlvName");
    ee.intName = args.str("EEIntName");
    ee.cryName = args.str("EECryName");

    ee.pFHalf = args.dble("EEPFHalf");
    ee.pFFifth = args.dble("EEPFFifth");
    ee.pF45 = args.dble("EEPF45");

    ee.vecEESCLims = args.vecDble("EESCLims");
    ee.iLength = args.dble("EEiLength");
    ee.iXYOff = args.dble("EEiXYOff");
    ee.cryZOff = args.dble("EECryZOff");
    ee.zFront = args.dble("EEzFront");

    //  Position supercrystals in EE Quadrant

    //********************************* cutbox for trimming edge SCs
    const double cutWid(ee.sCERear / sqrt(2.));
    ee.cutParms[0] = cutWid;
    ee.cutParms[1] = cutWid;
    ee.cutParms[2] = ee.sCELength / sqrt(2.);
    Solid eeCutBox = Box(ee.cutBoxName, ee.cutParms[0], ee.cutParms[1], ee.cutParms[2]);
    //**************************************************************

    const double zFix(ee.zFront - 3172.0_mm);  // fix for changing z offset

    //** fill supercrystal front and rear center positions from xml input
    for (unsigned int iC(0); iC != (unsigned int)ee.nSCquad; ++iC) {
      const unsigned int iOff(8 * iC);
      const unsigned int ix((unsigned int)ee.vecEESCCtrs[iOff + 0]);
      const unsigned int iy((unsigned int)ee.vecEESCCtrs[iOff + 1]);

      assert(ix > 0 && ix < 11 && iy > 0 && iy < 11);

      ee.scrFCtr[ix - 1][iy - 1] =
        DDTranslation(ee.vecEESCCtrs[iOff + 2], ee.vecEESCCtrs[iOff + 4], ee.vecEESCCtrs[iOff + 6] + zFix);

      ee.scrRCtr[ix - 1][iy - 1] =
        DDTranslation(ee.vecEESCCtrs[iOff + 3], ee.vecEESCCtrs[iOff + 5], ee.vecEESCCtrs[iOff + 7] + zFix);
    }

    //** fill crystal front and rear center positions from xml input
    for (unsigned int iC(0); iC != 25; ++iC) {
      const unsigned int iOff(8 * iC);
      const unsigned int ix((unsigned int)ee.vecEECRCtrs[iOff + 0]);
      const unsigned int iy((unsigned int)ee.vecEECRCtrs[iOff + 1]);

      assert(ix > 0 && ix < 6 && iy > 0 && iy < 6);

      ee.cryFCtr[ix - 1][iy - 1] =
        DDTranslation(ee.vecEECRCtrs[iOff + 2], ee.vecEECRCtrs[iOff + 4], ee.vecEECRCtrs[iOff + 6]);

      ee.cryRCtr[ix - 1][iy - 1] =
        DDTranslation(ee.vecEECRCtrs[iOff + 3], ee.vecEECRCtrs[iOff + 5], ee.vecEECRCtrs[iOff + 7]);
    }

    Solid eeCRSolid = Trap(ee.cryName,
			   0.5 * ee.crysLength,
			   atan((ee.crysRear - ee.crysFront) / (sqrt(2.) * ee.crysLength)),
			   45._deg,
			   0.5 * ee.crysFront,
			   0.5 * ee.crysFront,
			   0.5 * ee.crysFront,
			   0._deg,
			   0.5 * ee.crysRear,
			   0.5 * ee.crysRear,
			   0.5 * ee.crysRear,
			   0._deg);
    Volume eeCRLog = Volume(ee.cryName, eeCRSolid, ns.material(ee.crysMat));

    for (unsigned int isc(0); isc < ee.nSCTypes; ++isc) {
      unsigned int iSCType = isc + 1;
      const string anum(std::to_string(iSCType));
      const double eFront(0.5 * ee.sCEFront);
      const double eRear(0.5 * ee.sCERear);
      const double eAng(atan((ee.sCERear - ee.sCEFront) / (sqrt(2.) * ee.sCELength)));
      const double ffived(45._deg);
      const double zerod(0._deg);
      string eeSCEnvName(1 == iSCType ? ee.envName + std::to_string(iSCType)
			 : (ee.envName + std::to_string(iSCType) + "Tmp"));
      Solid eeSCEnv = ns.addSolidNS(
				    eeSCEnvName,
				    Trap(eeSCEnvName, 0.5 * ee.sCELength, eAng, ffived, eFront, eFront, eFront, zerod, eRear, eRear, eRear, zerod));

      const double aFront(0.5 * ee.sCAFront);
      const double aRear(0.5 * ee.sCARear);
      const double aAng(atan((ee.sCARear - ee.sCAFront) / (sqrt(2.) * ee.sCALength)));
      string eeSCAlvName(
			 (1 == iSCType ? ee.alvName + std::to_string(iSCType) : (ee.alvName + std::to_string(iSCType) + "Tmp")));
      Solid eeSCAlv = ns.addSolidNS(
				    eeSCAlvName,
				    Trap(eeSCAlvName, 0.5 * ee.sCALength, aAng, ffived, aFront, aFront, aFront, zerod, aRear, aRear, aRear, zerod));

      const double dwall(ee.sCAWall);
      const double iFront(aFront - dwall);
      const double iRear(iFront);
      const double iLen(ee.iLength);
      string eeSCIntName(1 == iSCType ? ee.intName + std::to_string(iSCType)
			 : (ee.intName + std::to_string(iSCType) + "Tmp"));
      Solid eeSCInt = ns.addSolidNS(eeSCIntName,
				    Trap(eeSCIntName,
					 iLen / 2.,
					 atan((ee.sCARear - ee.sCAFront) / (sqrt(2.) * ee.sCALength)),
					 ffived,
					 iFront,
					 iFront,
					 iFront,
					 zerod,
					 iRear,
					 iRear,
					 iRear,
					 zerod));

      const double dz(-0.5 * (ee.sCELength - ee.sCALength));
      const double dxy(0.5 * dz * (ee.sCERear - ee.sCEFront) / ee.sCELength);
      const double zIOff(-(ee.sCALength - iLen) / 2.);
      const double xyIOff(ee.iXYOff);

      Volume eeSCELog;
      Volume eeSCALog;
      Volume eeSCILog;

      if (1 == iSCType) {  // standard SC in this block
	eeSCELog = ns.addVolumeNS(Volume(myns + ee.envName + std::to_string(iSCType), eeSCEnv, ns.material(ee.mat)));
	eeSCALog = Volume(ee.alvName + std::to_string(iSCType), eeSCAlv, ns.material(ee.wallMat));
	eeSCILog = Volume(ee.intName + std::to_string(iSCType), eeSCInt, ns.material(ee.mat));
      } else {  // partial SCs this block: create subtraction volumes as appropriate
	const double half(ee.cutParms[0] - ee.pFHalf * ee.crysRear);
	const double fifth(ee.cutParms[0] + ee.pFFifth * ee.crysRear);
	const double fac(ee.pF45);

	const double zmm(0.0_mm);

	DDTranslation cutTra(
			     2 == iSCType ? DDTranslation(zmm, half, zmm)
			     : (3 == iSCType ? DDTranslation(half, zmm, zmm)
				: (4 == iSCType ? DDTranslation(zmm, -fifth, zmm)
				   : (5 == iSCType ? DDTranslation(-half * fac, -half * fac, zmm)
				      : DDTranslation(-fifth, zmm, zmm)))));

	const CLHEP::HepRotationZ cutm(ffived);

	Rotation3D cutRot(5 != iSCType ? Rotation3D()
			  : myrot(ns,
				  "EECry5Rot",
				  Rotation3D(cutm.xx(),
					     cutm.xy(),
					     cutm.xz(),
					     cutm.yx(),
					     cutm.yy(),
					     cutm.yz(),
					     cutm.zx(),
					     cutm.zy(),
					     cutm.zz())));

	Solid eeCutEnv = SubtractionSolid(ee.envName + std::to_string(iSCType),
					  ns.solid(ee.envName + std::to_string(iSCType) + "Tmp"),
					  eeCutBox,
					  Transform3D(cutRot, cutTra));

	const DDTranslation extra(dxy, dxy, dz);

	Solid eeCutAlv = SubtractionSolid(ee.alvName + std::to_string(iSCType),
					  ns.solid(ee.alvName + std::to_string(iSCType) + "Tmp"),
					  eeCutBox,
					  Transform3D(cutRot, cutTra - extra));

	const double mySign(iSCType < 4 ? +1. : -1.);

	const DDTranslation extraI(xyIOff + mySign * 2.0_mm, xyIOff + mySign * 2.0_mm, zIOff);

	Solid eeCutInt = SubtractionSolid(ee.intName + std::to_string(iSCType),
					  ns.solid(ee.intName + std::to_string(iSCType) + "Tmp"),
					  eeCutBox,
					  Transform3D(cutRot, cutTra - extraI));

	eeSCELog = ns.addVolumeNS(Volume(myns + ee.envName + std::to_string(iSCType), eeCutEnv, ns.material(ee.mat)));
	eeSCALog = Volume(ee.alvName + std::to_string(iSCType), eeCutAlv, ns.material(ee.wallMat));
	eeSCILog = Volume(ee.intName + std::to_string(iSCType), eeCutInt, ns.material(ee.mat));
      }
      eeSCELog.placeVolume(eeSCALog, iSCType * 100 + 1, Position(dxy, dxy, dz));
      eeSCALog.placeVolume(eeSCILog, iSCType * 100 + 1, Position(xyIOff, xyIOff, zIOff));

      DDTranslation croffset(0., 0., 0.);

      // Position crystals within parent supercrystal interior volume
      static const unsigned int ncol(5);

      if (iSCType > 0 && iSCType <= ee.nSCTypes) {
	const unsigned int icoffset((iSCType - 1) * ncol - 1);

	// Loop over columns of SC
	for (unsigned int icol(1); icol <= ncol; ++icol) {
	  // Get column limits for this SC type from xml input
	  const int ncrcol((int)ee.vecEESCProf[icoffset + icol]);

	  const int imin(0 < ncrcol ? 1 : (0 > ncrcol ? ncol + ncrcol + 1 : 0));
	  const int imax(0 < ncrcol ? ncrcol : (0 > ncrcol ? ncol : 0));

	  if (imax > 0) {
	    // Loop over crystals in this row
	    for (int irow(imin); irow <= imax; ++irow) {
	      // Create crystal as a DDEcalEndcapTrap object and calculate rotation and
	      // translation required to position it in the SC.
	      DDEcalEndcapTrap crystal(1, ee.crysFront, ee.crysRear, ee.crysLength);

	      crystal.moveto(ee.cryFCtr[icol - 1][irow - 1], ee.cryRCtr[icol - 1][irow - 1]);

	      string rname("EECrRoC" + std::to_string(icol) + "R" + std::to_string(irow));

	      eeSCALog.placeVolume(
				   eeCRLog,
				   100 * iSCType + 10 * (icol - 1) + (irow - 1),
				   Transform3D(
					       myrot(ns, rname, crystal.rotation()),
					       Position(crystal.centrePos().x(), crystal.centrePos().y(), crystal.centrePos().z() - ee.cryZOff)));
	    }
	  }
	}
      }
    }

    //** Loop over endcap columns
    for (int icol = 1; icol <= int(ee.nColumns); icol++) {
      //**  Loop over SCs in column, using limits from xml input
      for (int irow = int(ee.vecEEShape[2 * icol - 2]); irow <= int(ee.vecEEShape[2 * icol - 1]); ++irow) {
	if (ee.vecEESCLims[0] <= icol && ee.vecEESCLims[1] >= icol && ee.vecEESCLims[2] <= irow &&
	    ee.vecEESCLims[3] >= irow) {
	  // Find SC type (complete or partial) for this location
	  unsigned int isctype = 1;

	  for (unsigned int ii = 0; ii < (unsigned int)(ee.nSCCutaway); ++ii) {
	    if ((ee.vecEESCCutaway[3 * ii] == icol) && (ee.vecEESCCutaway[3 * ii + 1] == irow)) {
	      isctype = int(ee.vecEESCCutaway[3 * ii + 2]);
	    }
	  }

	  // Create SC as a DDEcalEndcapTrap object and calculate rotation and
	  // translation required to position it in the endcap.
	  DDEcalEndcapTrap scrys(1, ee.sCEFront, ee.sCERear, ee.sCELength);
	  scrys.moveto(ee.scrFCtr[icol - 1][irow - 1], ee.scrRCtr[icol - 1][irow - 1]);
	  scrys.translate(DDTranslation(0., 0., -ee.zOff));

	  string rname(ee.envName + std::to_string(isctype) + std::to_string(icol) + "R" + std::to_string(irow));
	  // Position SC in endcap
	  Volume quaLog = ns.volume(ee.quaName);
	  Volume childEnvLog = ns.volume(myns + ee.envName + std::to_string(isctype));
	  quaLog.placeVolume(childEnvLog,
			     100 * isctype + 10 * (icol - 1) + (irow - 1),
			     Transform3D(scrys.rotation(), scrys.centrePos()));
	}
      }
    }

    return 1;
  }
}

DECLARE_DDCMS_DETELEMENT(DDCMS_ecal_DDEcalEndcapAlgo, algorithm)
