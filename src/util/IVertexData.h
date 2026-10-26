/**
 * @file IVertexData.h
 *
 * @brief The IVertexData interface
 *
 * Interface for any class that stores and processes vertex attributes. Each attribute has a
 * unique name.
 *
 * @author Amit Shesh
 */
#ifndef UTIL_IVERTEXDATA_H
#define UTIL_IVERTEXDATA_H

#include <string>
#include <vector>

namespace util
{
    /**
     * Interface for any class that stores and processes vertex attributes.
     * Each attribute will have a unique name.
     */
    class IVertexData
    {
    public:
        virtual ~IVertexData() {}

        /**
         * Query if attribute by the given name is present
         *
         * @param attribName the name of the attribute that is being queried
         * @return true if data for this name is present, false otherwise
         */
        virtual bool hasData(const std::string &attribName) const = 0;

        /**
         * Returns the data for the supplied attribute name as a float array, for maximum
         * flexibility
         *
         * @param attribName the (unique) name of the attribute
         * @return the attribute data as a float array
         */
        virtual std::vector<float> getData(const std::string &attribName) const = 0;

        /**
         * Set the data for the given attribute. If the attribute is not already present, it will
         * be added now. If data is already present for the attribute, it will be overwritten
         *
         * @param attribName the name of the attribute to be added/set
         * @param data the data as an array of floats for maximum flexibility
         */
        virtual void setData(const std::string &attribName, const std::vector<float> &data) = 0;

        /**
         * Get a list of all the attribute names supported by an implementation
         *
         * @return the names of all supported attributes
         */
        virtual std::vector<std::string> getAllAttributes() const = 0;
    };
} // namespace util

#endif
