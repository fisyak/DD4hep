//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
//==========================================================================

#ifndef DDSEGMENTATION_BITFIELDCODER_H
#define DDSEGMENTATION_BITFIELDCODER_H 1

#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace dd4hep {

  namespace DDSegmentation {

    typedef int64_t  FieldID;
    typedef uint64_t CellID;
    typedef uint64_t VolumeID;

    class StringTokenizer ; 

    /// Helper class for BitFieldCoder that corresponds to one field value. 
    class BitFieldElement   {
  
    public :
      /// Default constructor
      BitFieldElement() = default ;
      /// Copy constructor
      BitFieldElement(const BitFieldElement&) = default ;
      /// Move constructor
      BitFieldElement(BitFieldElement&&) = default ;

      /** The standard c'tor.
       * @param  name          name of the field
       * @param  offset        offset of field
       * @param  signedWidth   width of field, negative if field is signed
       */
      BitFieldElement( const std::string& name, unsigned offset, int signedWidth ) ; 
      /// Default destructor
      ~BitFieldElement() = default ;

      /// Assignment operator
      BitFieldElement& operator=(const BitFieldElement&) = default ;

      /// calculate this field's value given an external 64 bit bitmap 
      FieldID value(CellID bitfield) const;

      // assign the given value to the bit field
      void set(CellID& bitfield, FieldID value) const ;

      /** The field's name */
      const std::string& name() const { return _name ; }

      /** The field's offset */
      unsigned offset() const { return _offset ; }

      /** The field's width */
      unsigned width() const  { return _width ; }

      /** True if field is interpreted as signed */
      bool isSigned() const   { return _isSigned ; }

      /** The field's mask */
      CellID mask() const     { return _mask ; }

      /** Minimal value  */
      int  minValue()  const  { return _minVal;  }

      /** Maximal value  */
      int  maxValue()  const  { return _maxVal;  }

    protected:
  
      CellID _mask      {};
      unsigned _offset  {};
      unsigned _width   {};
      int _minVal       {};
      int _maxVal       {};
      bool _isSigned    {};
      std::string _name;
    };


  
    /// Helper class for decoding and encoding a bit field of 64bits for convenient declaration
    /** and manipulation of sub fields of various widths.<br>
     *  This is a thread safe re-implementation of the functionality in the deprected BitField64.
     *  
     *  Example:<br>
     *    BitFieldCoder bc("layer:7,system:-3,barrel:3,theta:32:11,phi:11" ) ; <br> 
     *    bc.set( field,  "layer"  , 123 );         <br> 
     *    bc.set( field,  "system" , -4  );         <br> 
     *    bc.set( field,  "barrel" , 7   );         <br> 
     *    bc.set( field,  "theta"  , 180 );         <br> 
     *    bc.set( field,  "phi"    , 270 );         <br> 
     *    ...                                       <br>
     *    int theta = bc.get( field, "theta" ) ;                    <br>
     *    ...                                       <br>
     *    unsigned phiIndex = bc.index("phi") ;     <br>
     *    int phi = bc.get( field, phiIndex ) ;                <br>
     *
     *    @author F.Gaede, DESY
     *    @date  2017-09
     */  
    class BitFieldCoder{
    public :
      typedef std::map<std::string, unsigned int> IndexMap ;

    public :
      /// Default constructor
      BitFieldCoder() = default ;
      /// Copy constructor
      BitFieldCoder(const BitFieldCoder&) = default ;
      /// Move constructor
      BitFieldCoder(BitFieldCoder&&) = default ;
      /// Default destructor
      ~BitFieldCoder() = default ;

      /// Assignment operator
      BitFieldCoder& operator=(const BitFieldCoder&) = default ;
    
      /** The c'tor takes an initialization string of the form:<br>
       *  \<fieldDesc\>[,\<fieldDesc\>...]<br>
       *  fieldDesc = name:[start]:[-]length<br>
       *  where:<br>
       *  name: The name of the field<br>
       *  start: The start bit of the field. If omitted assumed to start 
       *  immediately following previous field, or at the least significant 
       *  bit if the first field.<br>
       *  length: The number of bits in the field. If preceeded by '-' 
       *  the field is signed, otherwise unsigned.<br>
       *  Bit numbering is from the least significant bit (bit 0) to the most 
       *  significant (bit 63). <br>
       *  Example: "layer:7,system:-3,barrel:3,theta:32:11,phi:11"
       */
      BitFieldCoder( const std::string& initString ) : _joined(0){
        init( initString ) ;
      }

      /** return a new 64bit value given as high and low 32bit words.
       */
      static CellID toLong(unsigned low_Word, unsigned high_Word ) {
        return (  ( low_Word & 0xffffffffULL ) |  ( ( high_Word & 0xffffffffULL ) << 32 ) ) ; 
      }
    
      /** The low  word, bits 0-31
       */
      static unsigned lowWord(CellID bitfield)  { return unsigned( bitfield &  0xffffFFFFUL ); } 

      /** The high  word, bits 32-63
       */
      static unsigned highWord(CellID bitfield) { return unsigned( bitfield >> 32); }

      /** get value of sub-field specified by index 
       */
      FieldID get(CellID bitfield, size_t idx) const { 
        return _fields.at(idx).value( bitfield )  ;
      }
    
      /** Access to field through name .
       */
      FieldID get(CellID bitfield, const std::string& name) const {
        return _fields.at( index( name ) ).value( bitfield ) ;
      }

      /** set value of sub-field specified by index 
       */
      void set(CellID& bitfield, size_t idx, FieldID value) const { 
        _fields.at(idx).set( bitfield , value )  ;
      }
    
      /** Access to field through name .
       */
      void set(CellID& bitfield, const std::string& name, FieldID value) const {
        _fields.at( index( name ) ).set( bitfield, value ) ;
      }

      /** Highest bit used in fields [0-63]
       */
      unsigned highestBit() const ;

      /** Number of values */
      size_t size() const { return _fields.size() ; }

      /** Index for field named 'name' 
       */
      size_t index( const std::string& name) const ;

      /** Const Access to field through name .
       */
      const BitFieldElement& operator[](const std::string& name) const { 
        return _fields[ index( name ) ] ;
      }

      /** Const Access to field through index .
       */
      const BitFieldElement& operator[](unsigned idx) const { 
        return _fields[ idx ] ;
      }

      /** Return a valid description string of all fields
       */
      std::string fieldDescription() const ;

      /** Return a string with a comma separated list of the current sub field values 
       */
      std::string valueString(CellID bitfield) const ;

      const std::vector<BitFieldElement>& fields()  const  {
        return _fields;
      }
    
      /** the mask of all the bits used in the description */
      CellID mask() const { return _joined ; }

    protected:

      /** Add an additional field to the list 
       */
      void addField( const std::string& name,  unsigned offset, int width ); 

      /** Decode the initialization string as described in the constructor.
       *  @see BitFieldCoder( const std::string& initString )
       */
      void init( const std::string& initString) ;

    protected:
      // -------------- data members:--------------
      std::vector<BitFieldElement> _fields{} ;
      IndexMap  _map{} ;
      CellID    _joined{} ;
    };


    /// Helper class  for string tokenization.
    /**  Usage:<br>
     *    std::vector<std::string> tokens ; <br>
     *    StringTokenizer t( tokens ,',') ; <br>
     *    std::for_each( aString.begin(), aString.end(), t ) ;  <br>
     *
     *    @author F.Gaede, DESY
     *    @date  2013-06
     */
    class StringTokenizer{
      std::vector< std::string >& _tokens ;
      char _del ;
      char _last ;

    public:
    
      /** Only c'tor, give (empty) token vector and delimeter character */
      StringTokenizer( std::vector< std::string >& tokens, char del ) 
        : _tokens(tokens) 
        , _del(del), 
          _last(del) {
      }
    
      /** Operator for use with algorithms, e.g. for_each */
      void operator()(const char& c) { 
        if( c != _del  ) {
          if( _last == _del  ) {
            _tokens.emplace_back("") ; 
          }
          _tokens.back() += c ;
        }
        _last = c ;
      } 
    };

  } // end namespace
} // end namespace
#endif




