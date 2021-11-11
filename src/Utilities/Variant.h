#pragma once
#include <cstdint>
#include <string>
#include <cassert>

enum eInterpolateType {
	INTERPOLATE_LINEAR = 0,
	INTERPOLATE_SMOOTHSTEP,
	INTERPOLATE_EASE_TO,
	INTERPOLATE_EASE_FROM,
	INTERPOLATE_EASE_TO_QUARTIC,
	INTERPOLATE_EASE_FROM_QUARTIC,
	INTERPOLATE_BOUNCE_TO,
	INTERPOLATE_SMOOTHSTEP_AS_COLOR,
	INTERPOLATE_LINEAR_AS_COLOR
};

#define C_VAR_SPACE_BYTES 16 // Enough to fit a rect

// Signals are ignored when copying these
class Variant {
public:
	enum eType {
		TYPE_UNUSED,
		TYPE_FLOAT,
		TYPE_STRING,
		TYPE_VECTOR2,
		TYPE_VECTOR3,
		TYPE_UINT32,
		TYPE_ENTITY,
		TYPE_COMPONENT,
		TYPE_RECT,
		TYPE_INT32
	};

	Variant(uint32_t var) { SetDefaults(); Set(var); }
	Variant(int32_t var) { SetDefaults(); Set(var); }
	Variant(float var) { SetDefaults(); Set(var); }
	Variant(const std::string& var) { SetDefaults(); Set(var); }
	Variant() { SetDefaults(); }
	~Variant();

	void Reset() { m_type = TYPE_UNUSED; }
	void Set(const Variant& v);
	void SetVariant(Variant* pVar) /* Needed this because boost was confused... */;

	void Set(float var) {
		assert(m_type == TYPE_UNUSED || m_type == TYPE_FLOAT);
		m_type = TYPE_FLOAT; *((float*)m_var) = var;
	}

	float& GetFloat() {
		if (m_type == TYPE_UNUSED) Set(float(0));
		assert(m_type == TYPE_FLOAT);
		return  *((float*)m_var);
	}

	const float& GetFloat() const {
		assert(m_type == TYPE_FLOAT);
		return  *((float*)m_var);
	}

	void Set(uint32_t var) {
		assert(m_type == TYPE_UNUSED || m_type == TYPE_UINT32);
		m_type = TYPE_UINT32; *((uint32_t*)m_var) = var;
	}

	uint32_t& GetUINT32() {
		if (m_type == TYPE_UNUSED) Set(uint32_t(0));
		//assert(m_type == TYPE_UINT32);
		return  *((uint32_t*)m_var);
	}

	const uint32_t& GetUINT32() const {
		assert(m_type == TYPE_UINT32);
		return  *((uint32_t*)m_var);
	}

	void Set(int32_t var) {
		assert(m_type == TYPE_UNUSED || m_type == TYPE_INT32);
		m_type = TYPE_INT32; *((int32_t*)m_var) = var;
	}

	int32_t& GetINT32() {
		if (m_type == TYPE_UNUSED) Set(int32_t(0));
		assert(m_type == TYPE_INT32);
		return  *((int32_t*)m_var);
	}

	const int32_t& GetINT32() const {
		assert(m_type == TYPE_INT32);
		return  *((int32_t*)m_var);
	}

	void Set(std::string const& var);

	std::string& GetString() {
		return m_string;
	}

	const std::string& GetString() const {
		return m_string;
	}


	eType GetType() const { return m_type; }
	std::string Print();


	Variant(const Variant& v)
	{
		SetDefaults();
		//our special copy won't include the sig crap, that stuff can't be copied
		*this = v;
	}


	Variant& operator= (const Variant& rhs)
	{
		m_type = rhs.m_type;
		m_pVoid = rhs.m_pVoid;
		memcpy(m_var, rhs.m_var, C_VAR_SPACE_BYTES);
		m_string = rhs.m_string;
		return *this;
	}

	/**
	 * Add-and-assign operator.
	 *
	 * Adds the value of \a rhs to the value of this \c Variant and assigns the new value to this.
	 * The type of this must match the type of \a rhs.
	 *
	 * The \c Variant type's specific addition operator is used. For example the
	 * \c CL_Vec2f::operator+() is used when adding two vectors together.
	 *
	 * This operator has effect only for \c Variant types where adding two values makes sense
	 * and is supported, such as the float and integer types. Adding for example two pointers
	 * together makes no sense so this operator has no effect for the \c TYPE_ENTITY nor
	 * \c TYPE_COMPONENT types among others.
	 *
	 * \note this operator is also supported for TYPE_STRING: it performs string
	 * catenation as expected.
	 * \return a reference to \c this.
	 */
	Variant& operator+=(const Variant& rhs);

	/**
	 * Subtract-and-assign operator.
	 *
	 * Subtracts the value of \a rhs from the value of this \c Variant and assigns the new value to this.
	 * The type of this must match the type of \a rhs.
	 *
	 * The \c Variant type's specific subtraction operator is used. For example the
	 * \c CL_Vec2f::operator-() is used when subtracting two vectors.
	 *
	 * This operator has effect only for \c Variant types where subtracting two values makes sense
	 * and is supported, such as the float and integer types. Subtracting for example two pointers
	 * from each other makes no sense so this operator has no effect for the \c TYPE_ENTITY nor
	 * \c TYPE_COMPONENT types among others.
	 *
	 * \return a reference to \c this.
	 */
	Variant& operator-=(const Variant& rhs);

	/**
	 * Equality operator.
	 *
	 * \c Variants are considered equal when their types and values are equal.
	 *
	 * If the type is \c TYPE_UNUSED then it doesn't matter what the values
	 * are, the \c Variants are considered equal.
	 *
	 * The values are compared for equality using the values' classes' own \c operator==().
	 * For example if the type is \c TYPE_VECTOR2 then \c CL_Vec2f::operator==() is used for
	 * comparing the values.
	 */
	bool operator==(const Variant& rhs) const;

	/**
	 * Inequality operator.
	 *
	 * The opposite of operator==().
	 *
	 * \see operator==() for a thorough explanation.
	 */
	bool operator!=(const Variant& rhs) const;

	void Interpolate(Variant* pA, Variant* pB, float curPos, eInterpolateType type);

	bool Save(FILE* fp, const std::string& varName); //assumes you've already fopen'ed something and pass the file pointer in

	void ClearConnections(); //clear any boost connections that were connections to its onchanged signal.

	friend class VariantList;

private:
	void SetDefaults() { m_type = TYPE_UNUSED; }
	eType m_type;
	void* m_pVoid;
	union {
		uint8_t  m_var[C_VAR_SPACE_BYTES]; //large enough so we can use the space for all the datatypes we care about

		//don't actually use these, these unions help me look at vars when debugging easier
		float m_as_floats[4];
		uint32_t m_as_uint32s[4];
		int32_t m_as_int32s[4];
	};

	std::string m_string;
};

/**
 * \relates Variant
 * Adds two \link Variant \c Variants \endlink together. Returns a new
 * \c Variant that contains the result of the operation.
 * \see Variant::operator+=()
 */
inline Variant operator+(Variant lhs, const Variant& rhs);

/**
 * \relates Variant
 * Subtracts \a rhs from \a lhs. Returns a new \c Variant that contains
 * the result of the operation.
 * \see Variant::operator-=()
 */
inline Variant operator-(Variant lhs, const Variant& rhs);

//a VariantList holds a group of variants, we pass these when we don't know in advance how many variants we want to use

#define C_MAX_VARIANT_LIST_PARMS 6

/*
//example of memory serialization of a VariantList
VariantList v;
v.Get(0).Set(uint32(42));
v.Get(1).Set("Hey guys");
//save to mem
byte *pData = v.SerializeToMem(&size, NULL);
//load from mem
VariantList b;
b.SerializeFromMem(pData, size);
//display
LogMsg("%s, the answer to life is %d", b.Get(1).GetString().c_str(), b.Get(0).GetUINT32());
//clean up the mem we initted earlier
SAFE_DELETE_ARRAY(pData);
*/

class VariantList {
public:
	VariantList() {};

	Variant& Get(int parmNum) { assert(parmNum < C_MAX_VARIANT_LIST_PARMS); return m_variant[parmNum]; }
	VariantList(Variant v0) { m_variant[0] = v0; }
	VariantList(Variant v0, Variant v1) { m_variant[0] = v0; m_variant[1] = v1; }
	VariantList(Variant v0, Variant v1, Variant v2) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; }
	VariantList(Variant v0, Variant v1, Variant v2, Variant v3) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; }
	VariantList(Variant v0, Variant v1, Variant v2, Variant v3, Variant v4) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; m_variant[4] = v4; }
	VariantList(Variant v0, Variant v1, Variant v2, Variant v3, Variant v4, Variant v5) { m_variant[0] = v0; m_variant[1] = v1; m_variant[2] = v2; m_variant[3] = v3; m_variant[4] = v4;  m_variant[5] = v5; }

	void Reset() {
		for (int i = 0; i < C_MAX_VARIANT_LIST_PARMS; i++) {
			m_variant[i].Reset();
		}
	}

	uint8_t* SerializeToMem(uint32_t* pSizeOut, uint8_t* pDest); // Pass in NULL for dest and it will new[] the memory itself
	bool SerializeFromMem(uint8_t* pSrc, int length, int* pBytesReadOut = NULL); //length seems to be unused..
	Variant m_variant[C_MAX_VARIANT_LIST_PARMS]; // Non-dynamic for speed
	void GetVariantListStartingAt(VariantList* pOut, int startIndex); // Shift variables over to the right, deleting some.  0 based index
	std::string GetContentsAsDebugString();
};
